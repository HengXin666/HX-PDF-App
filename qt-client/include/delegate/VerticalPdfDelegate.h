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
#ifndef _HX_VERTICAL_PDF_DELEGATE_H_
#define _HX_VERTICAL_PDF_DELEGATE_H_

#include <QStyledItemDelegate>

namespace HX {

class LazyPdfModel;

class VerticalPdfDelegate : public QStyledItemDelegate {
public:
    explicit VerticalPdfDelegate(LazyPdfModel* model, QObject* parent = nullptr);

    void paint(
        QPainter* painter, 
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const override;

    QSize sizeHint(
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const override;

private:
    LazyPdfModel* _model;
};

} // namespace HX

#endif // !_HX_VERTICAL_PDF_DELEGATE_H_