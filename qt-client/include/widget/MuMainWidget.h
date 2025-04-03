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
#ifndef _HX_MU_MAIN_WIDGET_H_
#define _HX_MU_MAIN_WIDGET_H_

#include <memory>
#include <vector>

#include <QWidget>
#include <QImage>

#include <mu/Document.h>
#include <utils/LRUCache.hpp>
namespace HX {

namespace Mu {

class PageRenderer; 

} // namespace Mu

/**
 * @brief MuPdf库的主显示界面控件
 */
class MuMainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MuMainWidget(QWidget* parent = nullptr);

    void setDocument(const QString& filePath);

Q_SIGNALS:
    /**
     * @brief 更新PDF位置信息
     * @param pageIndex 当前页面索引
     * @param totalPages 总页数
     * @param zoom 缩放倍率
     */
    void updatePdfPosInfo(int pageIndex, int totalPages, qreal zoom);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QSizeF pageSize(int num) {
        return _pageSizes[num] * _zoom;
    }

    /**
     * @brief 作废数据
     */
    void invalidate();

    /**
     * @brief 加载页面
     * @param page 页面索引
     * @param zoom 缩放倍率
     * @param image 图片
     */
    void loadPage(int page, float zoom, QImage image);

    HX::Mu::PageRenderer* _pageRenderer;
    std::unique_ptr<HX::Mu::Document> _doc;
    HX::LRUCache<int, QImage> _imgLRUCache;
    std::vector<QSizeF> _pageSizes; // 每一页的页面大小 (原始大小 * dpi缩放)
                                    // (为了适配某些画面大小不一样的pdf和计算高度)

    int _pageSpacing;   // 页面间距
    int _pageIndex;     // 页面索引
    int _totalPages;    // 总页数
    int _totalHeight;   // 总页面高度
    qreal _zoom;        // 缩放倍率
    qreal _dpi;         // 屏幕 dpi
    QPixmap _placeholderIcon; // 占位图片 (未加载时候显示的)
};

} // namespace HX

#endif // !_HX_MU_MAIN_WIDGET_H_