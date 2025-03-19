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
#ifndef _HX_MAIN_WINDOW_H_
#define _HX_MAIN_WINDOW_H_

#include <QMainWindow>

namespace HX {

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
protected:
    /**
     * @brief 监听根窗口事件, 如果改变了窗口状态, 就看看是否需要修改图标
     * @param obj 
     * @param event 
     * @return bool
     */
    bool eventFilter(QObject* obj, QEvent* event) override;
};

} // namespace HX

#endif // !_HX_MAIN_WINDOW_H_