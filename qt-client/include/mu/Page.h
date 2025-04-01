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
#ifndef _HX_PAGE_H_
#define _HX_PAGE_H_

#include <vector>

#include <QFont>
#include <QImage>

struct fz_page;
struct fz_display_list;

namespace HX::Mu {

class Document;

struct TextItem {
    QString text;  // 文字内容
    QRectF rect;   // 文字矩形区域
    QFont font;    // 字体信息
    QColor color;  // 字体颜色信息
    QPointF origin; // 文字的基线起点（origin）
};

class Page {
public:
    explicit Page(Document const& doc, int index) noexcept;

    /**
     * @brief 获取页面大小 (72 dpi)
     * 
     * @return QSizeF 
     */
    QSizeF size() const;

    /**
     * @brief 将页面渲染到QImage (包含绘图 和 文本)
     * @param scaleX X方向 缩放 (默认 = 1.f, > 1.f 放大, < 1.f 缩小)
     * @param scaleY Y方向 缩放 (默认 = 1.f, > 1.f 放大, < 1.f 缩小)
     * @param rotation 顺时针旋转的旋转度 (范围: [0.0f，360.0f))
     * @return QImage 如果失败, 则返回空的 QImage
     */
    QImage renderImage(float dpiX = 72.f, float dpiY = 72.f, float rotation = 0.0f) const;

    /**
     * @brief 将页面渲染到QImage (仅渲染绘图, 不包含文本)
     * @param scaleX X方向 缩放 (默认 = 1.f, > 1.f 放大, < 1.f 缩小)
     * @param scaleY Y方向 缩放 (默认 = 1.f, > 1.f 放大, < 1.f 缩小)
     * @param rotation 顺时针旋转的旋转度 (范围: [0.0f，360.0f))
     * @return QImage 如果失败, 则返回空的 QImage
     */
    QImage renderOnlyDraw(float scaleX = 1.0f, float scaleY = 1.0f, float rotation = 0.0f) const;

    std::vector<TextItem> testGetText() const;

    ~Page() noexcept;
private:
    Document const& _doc;
    mutable std::vector<TextItem> _textItems;

    fz_page* _page;
    fz_display_list* _displayList;
};

} // namespace HX::Mu

#endif // !_HX_PAGE_H_