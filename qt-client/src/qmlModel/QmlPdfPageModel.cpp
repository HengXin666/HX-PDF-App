#include <qmlModel/QmlPdfPageModel.h>

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QPaintEvent>
#include <QPainter>

#include <utils/FpsCount.hpp>

#include <mu/Page.h>
#include <mu/PageRenderer.h>
#include <mu/doc/NetRangeDocument.h>
#include <mu/stream/NetRangeStream.h>

namespace HX {

/// @todo 临时测试参数
static constexpr int DPI = 150;
static constexpr int DevicePixelRatio = 1;

QmlPdfPageModel::QmlPdfPageModel()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , _pageRenderer(new HX::Mu::PageRenderer{this})
    , _doc()
    , _imgLRUCache(10)
    , _pageSpacing(8)
    , _pageIndex(0)
    , _totalPages(0)
    , _totalHeight(0)
    , _zoom(1.)
    , _dpi(DPI)
    , _placeholderIcon()
{
    connect(_pageRenderer, &HX::Mu::PageRenderer::pageReady, this,
            &QmlPdfPageModel::loadPage);
}

void QmlPdfPageModel::setDocument(const QString& url) {
    qDebug() << "加载:" << url;
    auto tmpByteArr = url.toUtf8();
    const char* filePathCharPtr = tmpByteArr.constData();
    _doc = std::make_unique<HX::Mu::NetRangeDocument>(filePathCharPtr);
    _doc->setStream(HX::Mu::NetRangeStream::make()).buildDocument(filePathCharPtr);

    emit _pageRenderer->setDocument(_doc.get());

    _totalPages = _doc->pageCount();    
    _pageSizes.resize(_totalPages);

    for (int i = 0; i < _totalPages; ++i) {
        auto size = _doc->page(i)->size() * (_dpi / 72.f);
        _pageSizes[i] = std::move(size);
    }

    invalidate();

    emit totalPagesChanged(); // 告知这个值被修改了
}

QImage QmlPdfPageModel::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    int index = id.section("?", 0, 0).toInt();
    qDebug() << id << "requestedSize" << requestedSize << "index:" << index;
    if (!_imgLRUCache.contains(index)) {
        _pageRenderer->requestPage(index, _zoom, _dpi);
        QImage placeholder(pageSize(index).toSize(), QImage::Format_RGBA8888);
        placeholder.fill(Qt::black);
        if (size) {
            *size = placeholder.size();
        }
        return placeholder;
    }
    auto res = _imgLRUCache.get(index);
    if (size) {
        *size = res.size();
    }
    return res;
}

qreal QmlPdfPageModel::getPageHeight(int index) {
    qDebug() << "第" << index << "页高度:" << (_pageSizes[index] * (_zoom / DevicePixelRatio)).height();
    return (_pageSizes[index] * (_zoom / DevicePixelRatio)).height();
}

qreal QmlPdfPageModel::getPageWidth(int index) {
    qDebug() << "第" << index << "页宽度:" << (_pageSizes[index] * (_zoom / DevicePixelRatio)).width();
    return (_pageSizes[index] * (_zoom / DevicePixelRatio)).width();
}

QSizeF QmlPdfPageModel::pageSize(int index) {
    return _pageSizes[index] * (_zoom / DevicePixelRatio);
}

void QmlPdfPageModel::invalidate() {
    // 更新总高度
    _totalHeight = 0;
    for (int i = 0; i < _totalPages; ++i) {
        _totalHeight += pageSize(i).height() + _pageSpacing;
    }

    _imgLRUCache.clear();

    emit totalHeightChanged();
}

void QmlPdfPageModel::loadPage(int page, float zoom, QImage image) {
    static_cast<void>(zoom);
    if constexpr (DevicePixelRatio != 1) {
        image.setDevicePixelRatio(DevicePixelRatio);
    }
    _imgLRUCache.emplace(page, std::move(image));
    emit updataImage(page);
}

} // namespace HX