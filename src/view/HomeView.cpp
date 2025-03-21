#include <view/HomeView.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include <widget/TopBar.h>
#include <widget/SideBar.h>
#include <view/BookView.h>

HX::HomeView::HomeView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // 设置布局为无边距
    /*
        * 1. 顶部栏
        * 2. 侧边栏 + 主视图
    */
    HX::TopBar* _topBal = new HX::TopBar{this};
    _topBal->setMaximumHeight(100);
    layout->addWidget(_topBal);

    auto* hBoxLayout = new QHBoxLayout{};
    layout->addLayout(hBoxLayout);

    // === 侧边栏 === {
    auto* sideBar = new HX::SideBar{this};
    hBoxLayout->addWidget(sideBar); // 侧边栏

    // 样式
    sideBar->setFixedWidth(200);
    // } === 侧边栏 ===

    // === 主视图 === {
    auto* mainView = new QWidget{this};
    auto* mainLayout = new QVBoxLayout{mainView};
    hBoxLayout->addWidget(mainView);
    // } === 主视图 ===

    // 加载侧边栏可见项 (延迟加载)
    QTimer::singleShot(0, [sideBar, mainLayout] {
        auto switchView = [mainLayout](QWidget* newView) {
            // 清空 mainView 中已有的子控件
            QLayoutItem* child;
            while ((child = mainLayout->takeAt(0)) != nullptr) {
                delete child->widget(); // 删除 widget
                delete child;
            }
            // 添加新的视图
            mainLayout->addWidget(newView);
        };

        sideBar->addTopLevelItem("主页").addChild("书籍", [switchView] {
            auto* bookPage = new BookView;
            bookPage->setStyleSheet("background-color: lightblue;");
            switchView(bookPage);
        }).addChild("分类", [switchView] {
            auto* categoryPage = new QWidget;
            categoryPage->setStyleSheet("background-color: lightgreen;");
            switchView(categoryPage);
        }).addChild("已下载", [switchView] {
            auto* downloadPage = new QWidget;
            downloadPage->setStyleSheet("background-color: lightgray;");
            switchView(downloadPage);
        });
        
        auto bookShelf = sideBar->addTopLevelItem("书架");

        // 模拟网络加载, @todo 可能会多次调用, 以同步最新数据, 因此需要复用设计
        QTimer::singleShot(1000, [bookShelf = std::move(bookShelf)]() mutable {
            qDebug() << "网络加载完成";
            // 获取书架列表
            auto bookShelfList = QStringList{"书架1", "书架2", "书架3"};
            for (const auto& shelf : bookShelfList) {
                bookShelf.addChild(shelf, [] {
                    qDebug() << "书架";
                });
            }
        });
    });
}