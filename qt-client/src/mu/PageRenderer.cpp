#include <mu/PageRenderer.h>

#include <mu/Document.h>
#include <mu/Page.h>

namespace HX::Mu {

PageRenderer::PageRenderer(QObject* parent)
    : QThread(parent)
    , _doc(nullptr)
    , _pageNum(0)
    , _zoom(0)
    , _dpi(72)
{}

void PageRenderer::run() {
    if (!_doc) {
        return;
    }
    auto img = _doc->page(_pageNum)->renderImage(_dpi * _zoom);
    emit pageReady(_pageNum, _zoom, std::move(img));
}

void PageRenderer::requestPage(int page, float zoom, int dpi) {
    this->_pageNum = page;
    this->_zoom = zoom; // @todo 为什么 不使用 this, 直接 _zoom = zoom 会有bug?
    this->_dpi = dpi;
    start();
}

} // namespace HX::Mu