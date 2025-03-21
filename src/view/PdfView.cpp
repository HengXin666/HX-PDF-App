#include <view/PdfView.h>

#include <QVBoxLayout>

HX::PdfView::PdfView(QWidget* parent)
    : QWidget(parent)
    , _pdfView(new QPdfView{this})
    , _pdfDocument(new QPdfDocument{this})
{
    auto* layout = new QVBoxLayout{this};
    layout->addWidget(_pdfView);
    _pdfView->setDocument(_pdfDocument);
}

HX::PdfView::PdfView(const QString& pdfPath, QWidget* parent)
    : HX::PdfView(parent)
{
    _pdfDocument->load(pdfPath);
}