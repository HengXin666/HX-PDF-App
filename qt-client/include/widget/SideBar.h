#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-PDF-App.
 *
 * HX-PDF-App is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-PDF-App is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-PDF-App.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _HX_SIDE_BAR_H_
#define _HX_SIDE_BAR_H_

#include <QWidget>
#include <QTreeWidget>

namespace HX {

class SideBar : public QWidget {
    Q_OBJECT

    // 建造者类, 创建一级项后返回, 可以链式创建多个一级项的子项
    class Builder {
    public:
        Builder(SideBar* sideBar, QTreeWidgetItem* item)
            : _sideBar(sideBar)
            , _item(item)
        {}

        // 添加子项
        Builder& addChild(const QString& text, std::function<void()> fun) {
            auto* child = new QTreeWidgetItem{_item};
            child->setText(0, text);
            _sideBar->_itemFunMap[child] = fun;
            return *this;
        }
    private:
        SideBar* _sideBar;
        QTreeWidgetItem* _item;
    };
public:
    explicit SideBar(QWidget* parent = nullptr);

    // 添加一级项 (不能点击, 以触发事件)
    Builder addTopLevelItem(const QString& text) {
        auto* item = new QTreeWidgetItem{_treeWidget};
        item->setText(0, text);
        return Builder{this, item};
    }
private:
    QTreeWidget* _treeWidget;
    std::unordered_map<QTreeWidgetItem*, std::function<void()>> _itemFunMap;
};

} // namespace HX

#endif // !_HX_SIDE_BAR_H_