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
#ifndef _HX_VIEW_MANAGE_WIDGET_H_
#define _HX_VIEW_MANAGE_WIDGET_H_

#include <QWidget>
#include <QStackedWidget>

namespace HX {

class ViewManageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewManageWidget(QWidget* parent = nullptr);

private:
    friend class ViewManageProxy;

    // 页面栈, 当前控件永远只显示栈顶的控件
    QStackedWidget* _stackedWidget;
};

} // namespace HX

#endif // !_HX_VIEW_MANAGE_WIDGET_H_