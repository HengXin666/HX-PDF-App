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
#ifndef _HX_QML_PDF_PAGE_MODEL_H_
#define _HX_QML_PDF_PAGE_MODEL_H_

#include <memory>
#include <vector>

#include <QWidget>
#include <QImage>
#include <QQuickImageProvider>

#include <mu/Document.h>
#include <utils/LRUCache.hpp>

namespace HX {

namespace Mu {

class PageRenderer; 

} // namespace Mu

class QmlPdfPageModel : public QQuickImageProvider {
    Q_OBJECT

    explicit QmlPdfPageModel();
public:
    // 单例, 声明周期由 addImageProvider 管理
    static QmlPdfPageModel* get() {
        static QmlPdfPageModel* g = new QmlPdfPageModel{};
        return g;
    }

    /**
     * @brief 从url中加载文档
     * @param url 
     * @return void 
     */
    Q_INVOKABLE void setDocument(const QString& url);

    /**
     * @brief 渲染PDF为图片 (由qml处请求)
     * @param id url部分 (此处是 image://pdf/${index}?t=${Date.now()} 的 ${index}?t=${Date.now()} 部分)
     * @param size 大小
     * @param requestedSize 请求期望的大小
     * @return QImage 
     */
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    Q_INVOKABLE int getTotalPages() const { return _totalPages; }
    Q_INVOKABLE int getTotalHeight() const { return _totalHeight; }
    Q_INVOKABLE qreal getZoom() const { return _zoom; }

    Q_INVOKABLE void setZoom(qreal zoom) { 
        _zoom = zoom; 
        qDebug() << "当前" << _zoom;
        invalidate();
        emit updateZoom();
    }

    Q_INVOKABLE qreal getPageHeight(int index) const {
        if (index >= _pageSizes.size()) [[unlikely]] {
            return 320.0;
        }
        return pageSize(index).height();
    }
    Q_INVOKABLE qreal getPageWidth(int index) const {
        if (index >= _pageSizes.size()) [[unlikely]] {
            return 320.0;
        }
        return pageSize(index).width();
    }

Q_SIGNALS:
    void totalPagesChanged();
    void totalHeightChanged();
    void zoomChanged();

    void updateZoom();
    void updateImage(int index);

private:
    /**
     * @brief 获取 index 页的大小
     * @param index 
     * @return QSizeF 
     */
    QSizeF pageSize(int index) const;

    /**
     * @brief 作废数据
     */
    void invalidate();

    void loadPage(int page, float zoom, QImage image);

    HX::Mu::PageRenderer* _pageRenderer;
    std::unique_ptr<HX::Mu::Document> _doc;
    HX::ThreadSafeLRUCache<int, QImage> _imgLRUCache;
    std::vector<QSizeF> _pageSizes; // 每一页的页面大小 (原始大小 * dpi缩放)
                                    // (为了适配某些画面大小不一样的pdf和计算高度)

    int _pageSpacing;   // 页面间距
    int _pageIndex;     // 页面索引
    int _totalPages;    // 总页数
    int _totalHeight;   // 总页面高度
    qreal _zoom;        // 缩放倍率
    qreal _dpi;         // 屏幕 dpi
    QPixmap _placeholderIcon; // 占位图片 (未加载时候显示的)

    // 注册
    Q_PROPERTY(
        int _totalPages
        READ getTotalPages 
        NOTIFY totalPagesChanged
    );
    Q_PROPERTY(
        int _totalHeight
        READ getTotalHeight
        NOTIFY totalHeightChanged
    );
    Q_PROPERTY(
        qreal _zoom
        READ getZoom
        WRITE setZoom
        NOTIFY zoomChanged
    );
};

} // namespace HX

#endif // !_HX_QML_PDF_PAGE_MODEL_H_