#include <view/HomeView.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include <widget/TopBar.h>
#include <widget/SideBar.h>

HX::HomeView::HomeView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    /*
        * 1. 顶部栏
        * 2. 侧边栏 + 主视图
    */
    // layout->setContentsMargins(0, 0, 0, 0); // 设置布局边距
    // layout->setSpacing(0); // 设置布局间距

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
    
    // 可见项: 延迟加载
    QTimer::singleShot(0, [sideBar] {
        sideBar->addTopLevelItem("主页").addChild("书籍", [] {
            qDebug() << "书籍";
        }).addChild("分类", [] {
            qDebug() << "分类";
        }).addChild("已下载", [] {
            qDebug() << "已下载";
        });
        
        auto bookShelf = sideBar->addTopLevelItem("书架");

        // 模拟网络加载
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
        
    // } === 侧边栏 ===

    // 主视图
    auto* mainView = new QWidget{this};
    mainView->setMinimumWidth(600);
    mainView->setStyleSheet("background-color: rgb(255, 45, 45);");
    hBoxLayout->addWidget(mainView);
}