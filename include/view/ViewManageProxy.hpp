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
#ifndef _HX_VIEW_MANAGE_PROXY_H_
#define _HX_VIEW_MANAGE_PROXY_H_

#include <widget/ViewManageWidget.h>

namespace HX {

/**
 * @brief 界面管理代理类
 */
class ViewManageProxy {
public:
    void setMainDisplayBar(ViewManageWidget* m) {
        _viewManageWidget = m;
    }

    /**
        * @brief 添加页面并且显示
        * @param view 页面
        */
    void pushView(QWidget* view) {
        _viewManageWidget->_stackedWidget->addWidget(view);
        _viewManageWidget->_stackedWidget->setCurrentWidget(view);
    }

    /**
        * @brief 删除当前显示的页面, 然后显示之后的栈顶界面
        */
    void popView() {
        if (_viewManageWidget->_stackedWidget->currentIndex() > 0) {
            auto* delView = _viewManageWidget->_stackedWidget->currentWidget();
            _viewManageWidget->_stackedWidget->removeWidget(
                delView
            );
            delete delView;
        }
    }
private:
    ViewManageWidget* _viewManageWidget = nullptr;
};

} // namespace HX

#endif // !_HX_VIEW_MANAGE_PROXY_H_