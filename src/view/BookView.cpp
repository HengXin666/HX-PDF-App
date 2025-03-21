#include <view/BookView.h>

#include <QVBoxLayout>
#include <QListWidget>

#include <singleton/GlobalSingleton.hpp>
#include <view/PdfView.h>

HX::BookView::BookView(QWidget* parent)
    : QWidget(parent)
    , _bookList(new QListWidget{this})
{
    /*
        界面

        图片  图片
        书名  书名

        图片  
        书名  (可上下滑动, 左对齐)

        ...

        可以点击图片, 然后跳转到书籍详情页 (此处直接是跳转到浏览pdf界面(以测试pdf浏览))
    */

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(_bookList);
    _bookList->setViewMode(QListWidget::IconMode); // 显示模式
    _bookList->setIconSize({150, 225}); // 图标大小
    _bookList->setResizeMode(QListWidget::Adjust); // 调整模式
    _bookList->setUniformItemSizes(true); // 统一大小
    _bookList->setSpacing(10); // 间距
    _bookList->setMovement(QListView::Static); // 禁止移动

    // 测试数据
    for (int i = 0; i < 10; ++i) {
        auto* item = new QListWidgetItem{_bookList};
        item->setIcon(QIcon{":/imgs/book.png"});
        item->setText("书籍名称");
        item->setData(Qt::UserRole, "D:/command/编程学习资料/04、GitHub使用帮助.pdf");
        item->setTextAlignment(Qt::AlignHCenter);
    }

    // 点击元素, 会打开书籍详情页 (此处直接是打开pdf浏览界面)
    connect(_bookList, &QListWidget::itemClicked, 
        [this](QListWidgetItem* item) {
        // 打开pdf浏览界面
        HX::GlobalSingleton::get().viewManageProxy.pushView(
            new HX::PdfView{item->data(Qt::UserRole).toString()}
        );
    });
}