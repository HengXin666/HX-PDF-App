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
#ifndef _HX_PDF_VIEW_H_
#define _HX_PDF_VIEW_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPdfView;
class QPdfDocument;
class QLabel;
QT_END_NAMESPACE

namespace HX {

class LeftDirectoryBar;

/**
 * @brief PDF 预览界面
 */
class PdfView : public QWidget {
    Q_OBJECT

    friend LeftDirectoryBar;
public:
    explicit PdfView(QWidget* parent = nullptr);
    explicit PdfView(const QString& pdfPath, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    QPdfView* _pdfView;
    QPdfDocument* _pdfDocument;
    LeftDirectoryBar* _leftDirectoryBar; // 左侧边目录栏
    QLabel* _totalPage; // 总页数
};

} // namespace HX

#endif // !_HX_PDF_VIEW_H_