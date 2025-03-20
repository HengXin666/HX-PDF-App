#include <widget/SideBar.h>

#include <QVBoxLayout>
#include <QTreeWidgetItem>

#include <unordered_map>
#include <functional>

HX::SideBar::SideBar(QWidget* parent)
    : QWidget(parent)
    , _treeWidget(new QTreeWidget{this})
    , _itemFunMap()
{
    _treeWidget->setHeaderHidden(true);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(_treeWidget);
    
    // 对于动态更新, 从http中解析后
    // 动态添加, 然后对应的函数传参是这个的url什么的

    connect(_treeWidget, &QTreeWidget::itemClicked, this,
        [this](QTreeWidgetItem* item, int) {
        if (item) {
            auto it = _itemFunMap.find(item);
            if (it != _itemFunMap.end()) {
                it->second();
            }
        }
    });
}