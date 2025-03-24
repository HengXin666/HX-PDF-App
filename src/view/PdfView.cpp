#include <view/PdfView.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#include <QLabel>

#include <widget/SvgIconPushButton.h>
#include <widget/LeftDirectoryBar.h>

HX::PdfView::PdfView(QWidget* parent)
    : QWidget(parent)
    , _pdfView(new QPdfView{this})
    , _pdfDocument(new QPdfDocument{this})
{
    /*
        [展开侧边栏]         [缩小][放大][自适应] 当前页(可输入)/总页数

        (可能的侧边栏)       <pdf预览>
    */
    auto* layout = new QVBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0); // 设置布局为无边距

    // === 上边栏 === {
    auto* topLayout = new QHBoxLayout{};
    layout->addLayout(topLayout);

    // 侧边栏按钮 (显示/隐藏侧边栏)
    auto* btnSidebar = new HX::SvgIconPushButton{
        ":/icons/menu.svg", "#FFFFFF", "#990099", this
    };
    btnSidebar->setFixedSize({32, 32});
    topLayout->addWidget(btnSidebar);
    topLayout->addStretch();

    // 上边栏 中间操作部分
    auto* topMiddleLayout = new QHBoxLayout{};
    topLayout->addLayout(topMiddleLayout);

    // 缩小按钮
    auto* btnZoomOut = new HX::SvgIconPushButton{
        ":/icons/minus.svg", "#FFFFFF", "#990099", this
    };
    btnZoomOut->setFixedSize({32, 32});
    topMiddleLayout->addWidget(btnZoomOut);

    // 放大按钮
    auto* btnZoomIn = new HX::SvgIconPushButton{
        ":/icons/add.svg", "#FFFFFF", "#990099", this
    };
    btnZoomIn->setFixedSize({32, 32});
    topMiddleLayout->addWidget(btnZoomIn);

    // 自适应按钮
    auto* btnFit = new HX::SvgIconPushButton{
        ":/icons/adaptive.svg", "#FFFFFF", "#990099", this
    };
    btnFit->setFixedSize({32, 32});
    topMiddleLayout->addWidget(btnFit);

    // 当前页面 (可输入) 页数
    _currentPage = new QLineEdit{this};
    _currentPage->setFixedWidth(50);
    _currentPage->setAlignment(Qt::AlignCenter);
    _currentPage->setValidator(new QIntValidator{this});
    topMiddleLayout->addWidget(_currentPage);

    // 分界
    auto* separator = new QLabel{"/", this};
    separator->setFixedWidth(10);
    separator->setAlignment(Qt::AlignCenter);
    topMiddleLayout->addWidget(separator);

    // 总页数
    _totalPage = new QLabel{"0 页", this}; // 总页数
    _totalPage->setFixedWidth(50);
    _totalPage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topMiddleLayout->addWidget(_totalPage);

    topLayout->addStretch();

    // 放大
    connect(btnZoomIn, &QPushButton::clicked, this, [this](){
        qreal cdelta = _pdfView->zoomFactor();
        _pdfView->setZoomFactor(cdelta * 1.1);
    });

    // 缩小
    connect(btnZoomOut, &QPushButton::clicked, this, [this](){
        qreal cdelta = _pdfView->zoomFactor();
        _pdfView->setZoomFactor(cdelta * 0.9);
    });

    // 自适应
    connect(btnFit, &QPushButton::clicked, this, [this](){
        if (!_pdfView->document() || _pdfView->document()->pageCount() == 0) 
            return;
        // 参考: https://forum.qt.io/topic/157002/qpdfview-why-doessn-t-this-work/3

        // 获取屏幕逻辑 DPI
        qreal dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();

        // 获取当前 PDF 第一页的宽度 (单位: pt)
        qreal pageWidthPt = _pdfView->document()->pagePointSize(0).width();
        if (pageWidthPt <= 0) 
            return;

        // 计算 PDF 页面在像素单位下的实际宽度
        qreal pageWidthPx = (pageWidthPt / 72.0) * dpi;

        // 获取 QPdfView 视口宽度 (单位: px)
        int viewportWidth = _pdfView->viewport()->width();
        if (viewportWidth <= 0) 
            return;

        // 计算合适的缩放因子
        constexpr qreal safetyMargin = 0.96;  // 适当 4% 的偏差
        qreal zoomFactor = (viewportWidth * safetyMargin) / pageWidthPx;

        _pdfView->setZoomFactor(zoomFactor);
    });

    auto* nav = _pdfView->pageNavigator();

    // 滚动后触发 更新当前页面
    connect(nav, &QPdfPageNavigator::currentPageChanged, this,
        [this](int index) {
        _currentPage->setText(QString{"%1"}.arg(index + 1));
    });

    // 回车后, 跳转到界面
    connect(_currentPage, &QLineEdit::returnPressed, this,
        [this, nav](){
        int page = _currentPage->text().toInt();
        if (page <= 0 || page > _pdfDocument->pageCount()) {
            _currentPage->setText(QString{"%1"}.arg(nav->currentPage() + 1));
            return;
        }
        nav->update(page - 1, {}, nav->currentZoom());
    });

    // } === 上边栏 ===

    auto* mainContentWidget = new QWidget{this};
    auto* contentLayout = new QHBoxLayout{mainContentWidget};
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);


    // === pdf 渲染设置 ===

    // pdf 预览
    _pdfView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _pdfView->setDocument(_pdfDocument);
    // 设置 PDF 查看模式为多页模式
    _pdfView->setPageMode(QPdfView::PageMode::MultiPage);

    // 设置自定义的缩放模式
    _pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

    // 去掉边距
    _pdfView->setContentsMargins(0, 0, 0, 0);
    _pdfView->viewport()->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(_pdfView);

    layout->addWidget(mainContentWidget);

    // === 侧边栏 === {
    _leftDirectoryBar = new HX::LeftDirectoryBar{200, this, mainContentWidget};
    _leftDirectoryBar->setGeometry(-200, 0, 200, height() - _totalPage->height());
    _leftDirectoryBar->updateHeight(height() - _totalPage->height());

    // 展开/关闭侧边栏
    connect(btnSidebar, &QPushButton::clicked, this,
        [this]() {
        _leftDirectoryBar->toggle();
    });
    // } === 侧边栏 ===
}

HX::PdfView::PdfView(const QString& pdfPath, QWidget* parent)
    : HX::PdfView(parent)
{
    _pdfDocument->load(pdfPath);
    _totalPage->setText(QString{"%1 页"}.arg(_pdfDocument->pageCount()));
    _currentPage->setText(QString{"%1"}.arg(_pdfView->pageNavigator()->currentPage() + 1));
}

void HX::PdfView::resizeEvent(QResizeEvent*) {
    if (_leftDirectoryBar) {
        _leftDirectoryBar->updateHeight(height() - _totalPage->height());
    }
}