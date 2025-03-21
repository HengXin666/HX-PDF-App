#include <view/PdfView.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>

#include <widget/SvgIconPushButton.h>

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
        ":/icons/menu.svg", "#0", "#990099", this
    };
    topLayout->addWidget(btnSidebar);
    topLayout->addStretch();

    // 上边栏 中间操作部分
    auto* topMiddleLayout = new QHBoxLayout{};
    topLayout->addLayout(topMiddleLayout);

    // 缩小按钮
    auto* btnZoomOut = new HX::SvgIconPushButton{
        ":/icons/minus.svg", "#FFFFFF", "#990099", this
    };
    topMiddleLayout->addWidget(btnZoomOut);

    // 放大按钮
    auto* btnZoomIn = new HX::SvgIconPushButton{
        ":/icons/add.svg", "#FFFFFF", "#990099", this
    };
    topMiddleLayout->addWidget(btnZoomIn);

    // 自适应按钮
    auto* btnFit = new HX::SvgIconPushButton{
        ":/icons/adaptive.svg", "#FFFFFF", "#990099", this
    };
    topMiddleLayout->addWidget(btnFit);

    // 当前页面 (可输入) 页数
    auto* currentPage = new QLineEdit{this};
    currentPage->setFixedWidth(50);
    currentPage->setAlignment(Qt::AlignCenter);
    topMiddleLayout->addWidget(currentPage);

    // 分界
    auto* separator = new QLabel{"/", this};
    separator->setFixedWidth(10);
    separator->setAlignment(Qt::AlignCenter);
    topMiddleLayout->addWidget(separator);

    // 总页数
    _totalPage = new QLabel{"0", this}; // 总页数
    _totalPage->setFixedWidth(50);
    _totalPage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topMiddleLayout->addWidget(_totalPage);

    topLayout->addStretch();
    // } === 上边栏 ===

    auto* mainContentWidget = new QWidget{this};
    auto* contentLayout = new QHBoxLayout{mainContentWidget};
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // pdf 预览
    _pdfView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _pdfView->setDocument(_pdfDocument);
    contentLayout->addWidget(_pdfView);

    layout->addWidget(mainContentWidget);

    // === 侧边栏 === {
    _leftDirectoryBar = new HX::LeftDirectoryBar{200, mainContentWidget};
    _leftDirectoryBar->setGeometry(-200, 0, 200, height() - _totalPage->height());
    _leftDirectoryBar->updateHeight(height() - _totalPage->height());

    connect(btnSidebar, &QPushButton::clicked, this,
        [this]() {
        qDebug() << "Toggle Sidebar" << _leftDirectoryBar->rect() << "{";
        _leftDirectoryBar->toggle();
        qDebug() << "} // Toggle Sidebar:" << _leftDirectoryBar->rect();
    });
    // } === 侧边栏 ===
}

HX::PdfView::PdfView(const QString& pdfPath, QWidget* parent)
    : HX::PdfView(parent)
{
    _pdfDocument->load(pdfPath);
}