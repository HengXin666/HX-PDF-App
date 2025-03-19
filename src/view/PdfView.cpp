#include <view/PdfView.h>

HX::PdfView::PdfView(QWidget* parent)
    : QWidget(parent)
    , _pdfView(new QPdfView{this})
    , _pdfDocument(new QPdfDocument{this})
{
    
}