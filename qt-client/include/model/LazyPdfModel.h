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
#ifndef _HX_LAZY_PDF_MODEL_H_
#define _HX_LAZY_PDF_MODEL_H_

#include <QAbstractListModel>
#include <QPixmap>
#include <QCache>

QT_BEGIN_NAMESPACE
class QPdfDocument;
class QPdfPageRenderer;
QT_END_NAMESPACE

namespace HX {

/**
 * @brief 懒加载PDF类
 */
class LazyPdfModel : public QAbstractListModel {
    Q_OBJECT
public:
    /**
     * @brief Construct a new Lazy Pdf Model object
     * @param cnt 总页面数
     * @param document 文档
     * @param parent 
     */
    explicit LazyPdfModel(QPdfDocument* document, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _cnt;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DecorationRole) const override;

    /**
     * @brief 预加载当前可视区域 (带一定的边界) [start, end + margin]
     * @param start 
     * @param end 
     * @param margin 预加载偏移
     */
    void preloadVisibleArea(int start, int end, int margin = 1);

    void updateVisibleRange(int start, int end, int margin = 1) {
        _visibleRange = {start, end};
    }

    void setScaleFactor(double factor) {
        _zoomFactor = factor;
        _imageCache.clear();  // 缩放时重新加载
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::DecorationRole});
    }

    QSize getSize() const {
        auto [w, h] = _baseSize;
        return QSize{
            static_cast<int>(w * _zoomFactor),
            static_cast<int>(h * _zoomFactor),
        };
    }

Q_SIGNALS:
    void needUpdate();

private:
    /**
     * @brief 加载图片
     * @param row 行数
     */
    void loadImage(int row);

    int _cnt; // 总页数
    float _zoomFactor{1.f}; // 缩放因子
    QSizeF _baseSize;  // 基础画面大小
    QPixmap _placeholder; // 占位图片 (未加载时候的临时显示)
    mutable QCache<int, QPixmap> _imageCache;
    QSet<int> _pendingLoads;
    QPdfPageRenderer* _renderer;
    struct VisibleRange {
        int l, r;
    } _visibleRange{0, 0};
};

} // namespace HX

#endif // !_HX_LAZY_PDF_MODEL_H_