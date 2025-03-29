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
#ifndef _HX_TOP_BAR_H_
#define _HX_TOP_BAR_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

namespace HX {

/**
* @brief 顶部栏
*/
class TopBar : public QWidget {
    Q_OBJECT
public:
    explicit TopBar(QWidget* parent = nullptr);
    
private:
    /**
    * @brief 还原窗口
    */
    void updateMaximizeIcon();

    // 如果存在可回退界面, 则显示该按钮
    QPushButton* _btnPop;

    // 搜索栏
    QPushButton* _btnSearch;
    QLineEdit* _textSearch;

    // 头像 用户名 等级
    QLabel* _imgAvatar;
    QLabel* _textUsername;
    QLabel* _textLevel;

    // 消息 更多(设置/...反正是个选项卡)
    QPushButton* _btnMsg;
    QToolButton* _toolBtn;

    // 隐藏 最大化 关闭
    QPushButton* _btnHide;
    QPushButton* _btnMaximize;
    QPushButton* _btnClose;
};

} // namespace HX

#endif // !_HX_TOP_BAR_H_