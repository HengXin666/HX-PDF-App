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
#ifndef _HX_LEFT_DIRECTORY_BAR_H_
#define _HX_LEFT_DIRECTORY_BAR_H_

#include <QWidget>
#include <QPropertyAnimation>



namespace HX {

/**
 * @brief 左侧边目录栏 (带动画)
 */
class LeftDirectoryBar : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param w 控件宽度
     * @param parent 父对象
     */
    LeftDirectoryBar(int w, class PdfView* pdfView, QWidget* parent = nullptr);

    /**
     * @brief 切换侧边栏显示状态
     */
    void toggle();

    /**
     * @brief 更新控件高度
     * @param h 高度
     */
    void updateHeight(int h) {
        int sidebarX = _visible ? 0 : -width();
        setGeometry(sidebarX, 0, width(), h);
    }

    bool getVisible() const {
        return _visible;
    }

private:
    QPropertyAnimation* _animation;
    bool _visible = true;
};

} // namespace HX

#endif // !_HX_LEFT_DIRECTORY_BAR_H_