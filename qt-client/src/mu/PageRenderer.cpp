#include <mu/PageRenderer.h>

#include <mu/Document.h>
#include <mu/Page.h>

namespace HX::Mu {

PageRenderer::PageRenderer(QObject* parent)
    : QThread(parent)
    , _doc(nullptr)
    , _pageNum(0)
    , _zoom(0)
{}

void PageRenderer::run() {
    if (!_doc) {
        return;
    }
    auto img = _doc->page(_pageNum)->renderImage(72 * _zoom);
    emit pageReady(_pageNum, _zoom, std::move(img));
}

void PageRenderer::requestPage(int page, float zoom) {
    _pageNum = page;
    _zoom = zoom;
    start();
}

} // namespace HX::Mu