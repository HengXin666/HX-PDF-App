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
#ifndef _HX_PDF_LIST_VIEW_H_
#define _HX_PDF_LIST_VIEW_H_

#include <QListView>

QT_BEGIN_NAMESPACE
class QPropertyAnimation;
QT_END_NAMESPACE

namespace HX {

class PdfListView : public QListView {
    Q_OBJECT
public:
    explicit PdfListView(QWidget* parent = nullptr);

    double zoomFactor() const {
        return _zoomFactor;
    }

    void setZoomFactor(double zoomFactor) {
        _zoomFactor = zoomFactor;
        emit scaleChanged(_zoomFactor);
    }

    int getVisibleIndex() const;

protected:
    void wheelEvent(QWheelEvent* event) override;

Q_SIGNALS:
    void scaleChanged(double factor);

private:
    double _zoomFactor = 1.0;
    QPropertyAnimation* _barAnimation;
};

} // namespace HX

#endif // !_HX_PDF_LIST_VIEW_H_