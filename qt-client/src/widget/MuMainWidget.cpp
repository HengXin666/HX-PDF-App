#include <widget/MuMainWidget.h>

#include <QGuiApplication>
#include <QScreen>
#include <QPaintEvent>

#include <mu/Document.h>
#include <mu/Page.h>
#include <mu/PageRenderer.h>

namespace HX {

MuMainWidget::MuMainWidget(QWidget* parent)
    : QWidget(parent)
    , _pageRenderer(new HX::Mu::PageRenderer{this})
    , _doc()
    , _imgCache()
    , _pageSpacing(8)
    , _pageIndex(0)
    , _totalPages(0)
    , _totalHeight(0)
    , _zoom(1.)
    , _dpi(QGuiApplication::primaryScreen()->logicalDotsPerInch())
{}

void MuMainWidget::setDocument(const QString& filePath) {
    _doc = std::make_unique<HX::Mu::Document>(filePath.toUtf8().constData());
    // todo 设置自定义网络流

    _totalPages = _doc->pageCount();    
    _pageSizes.resize(_totalPages);

    for (int i = 0; i < _totalPages; ++i) {
        auto size =  _doc->page(i)->size() * _dpi / 72.f;
        _pageSizes[i] = std::move(size);
    }

    invalidate();
}

void MuMainWidget::paintEvent(QPaintEvent* event) {

}

void MuMainWidget::invalidate() {
    // 更新总高度
    _totalHeight = 0;
    for (int i = 0; i < _totalHeight; ++i) {
        _totalHeight += pageSize(i).height() + _pageSpacing;
    }

    setMinimumHeight(_totalHeight);
    _imgCache.clear();
    update();
}

} // namespace HX