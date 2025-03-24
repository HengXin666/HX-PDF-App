#include <widget/LeftDirectoryBar.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QPdfBookmarkModel>
#include <QPdfPageNavigator>
#include <QPdfDocument>
#include <QPdfView>
#include <QPropertyAnimation>
#include <QStandardItemModel>

#include <view/PdfView.h>

namespace HX {

LeftDirectoryBar::LeftDirectoryBar(int w, PdfView* pdfView, QWidget* parent) 
    : QWidget(parent)
    , _animation(new QPropertyAnimation{this, "geometry"})
{
    setFixedWidth(w); // 侧边栏固定宽度
    auto* layout = new QVBoxLayout{this};

    // 设置动画持续时间
    _animation->setDuration(300);

    // 设置动画曲线
    _animation->setEasingCurve(QEasingCurve::InOutSine);

    auto* tabWidget = new QTabWidget{this};
    tabWidget->setTabPosition(QTabWidget::West);
    layout->addWidget(tabWidget);

    // 目录视图
    auto* bookView = new QTreeView{this};
    tabWidget->addTab(bookView, QIcon{":/icons/menu.svg"}, "");
    bookView->setHeaderHidden(true);
    tabWidget->setTabToolTip(0, "列表目录");

    // 创建书签模型, 并设置到书签视图
    auto* bookmodel = new QPdfBookmarkModel;
    bookmodel->setDocument(pdfView->_pdfDocument);
    bookView->setModel(bookmodel);

    // 图片视图
    auto* imgView = new QListView{this};
    tabWidget->addTab(imgView, QIcon{":/icons/picture.svg"}, "");
    tabWidget->setTabToolTip(1, "缩略图目录");

    imgView->setDragEnabled(false);
    imgView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    imgView->setSelectionBehavior(QAbstractItemView::SelectItems);
    imgView->setSelectionMode(QAbstractItemView::SingleSelection);

    imgView->setViewMode(QListView::IconMode);
    imgView->setSpacing(5);
    imgView->setResizeMode(QListView::Adjust);
    
    // 设置图片视图的模型
    imgView->setModel(pdfView->_pdfDocument->pageModel());
    imgView->setItemAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    auto* nav = pdfView->_pdfView->pageNavigator();
    
    // 点击树状列表目录事件
    connect(bookView, &QTreeView::clicked, this,
        [this, bookmodel, nav](const QModelIndex& index) {
        int jumpPage = bookmodel->data(index, int(QPdfBookmarkModel::Role::Page)).toInt();
        QPointF jumppos = bookmodel->data(index, int(QPdfBookmarkModel::Role::Location)).toPointF();
        nav->update(jumpPage, jumppos, nav->currentZoom());
    });

    // 点击缩略图目录事件
    connect(imgView, &QListWidget::clicked, this,
    [this, imgView, nav](const QModelIndex& index) {
        nav->update(index.row(), {}, nav->currentZoom());
    });
}

void LeftDirectoryBar::toggle() {
    int startX = _visible ? 0 : -width();
    int endX = _visible ? -width() : 0;
    _animation->setStartValue(QRect(startX, 0, width(), height()));
    _animation->setEndValue(QRect(endX, 0, width(), height()));
    _animation->start();
    _visible = !_visible;
}

} // namespace HX