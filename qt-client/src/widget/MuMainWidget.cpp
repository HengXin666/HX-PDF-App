#include <widget/MuMainWidget.h>

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

/**
 * @brief 一些配置 (调试使用), 目前无法保证以后会怎么样. 因为渲染卡顿的问题, 依旧存在...
 */

static constexpr int DPI = 150;
static constexpr int DevicePixelRatio = 1;

MuMainWidget::MuMainWidget(QWidget* parent)
    : QWidget(parent)
    , _pageRenderer(new HX::Mu::PageRenderer{this})
    , _doc()
    , _imgLRUCache(10)
    , _pageSizes()
    , _pageSpacing(8)
    , _pageIndex(0)
    , _totalPages(0)
    , _totalHeight(0)
    , _zoom(1.)
    , _dpi(DPI)
    , _placeholderIcon()
{
    connect(_pageRenderer, &HX::Mu::PageRenderer::pageReady,
            this, &MuMainWidget::loadPage);
    grabGesture(Qt::SwipeGesture);
}

void MuMainWidget::setDocument(const QString& filePath) {
    auto tmpByteArr = filePath.toUtf8();
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
}

void MuMainWidget::paintEvent(QPaintEvent* event) {
    // debug
    // static FpsCount fps;
    // if (auto cnt = fps.count())
    //     qDebug() << "fps:" << *cnt;

    QPainter painter{this};

    // 找到需要渲染的第一页
    int page = 0;
    int y = 0;

    while (page < _totalPages) {
        int height = pageSize(page).toSize().height();
        if (y + height >= event->rect().top()) {
            break;
        }
        y += _pageSpacing + height;
        ++page;
    }
    y += _pageSpacing;
    _pageIndex = page;

    // 渲染实际的界面
    while (y < event->rect().bottom() && page < _totalPages) {
        QSizeF size = pageSize(page);

        if (_imgLRUCache.contains(page)) {
            const auto& img = _imgLRUCache.get(page);
            // 很卡, 但是是我们需要的! (目前无法解决这个破烂问题...)
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.drawImage((width() - size.width()) / 2, y, img);
            emit updatePdfPosInfo(_pageIndex, _totalPages, _zoom);
        } else {
            painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
            painter.fillRect((width() - size.width()) / 2, y, size.width(),
                             size.height(), Qt::white);
            // painter.drawPixmap((size.width() - _placeholderIcon.width()) / 2,
            //                    (size.height() - _placeholderIcon.height()) / 2,
            //                    _placeholderIcon);
            _pageRenderer->requestPage(page, _zoom, _dpi);
        }
        y += size.height() + _pageSpacing;
        ++page;
    }
}

QSizeF MuMainWidget::pageSize(int num) {
    return _pageSizes[num] * (_zoom / DevicePixelRatio);
}

void MuMainWidget::invalidate() {
    // 更新总高度
    _totalHeight = 0;
    for (int i = 0; i < _totalPages; ++i) {
        _totalHeight += pageSize(i).height() + _pageSpacing;
    }

    setMinimumHeight(_totalHeight);
    _imgLRUCache.clear();
    update();
}

void MuMainWidget::loadPage(int page, float zoom, QImage image) {
    static_cast<void>(zoom);
    if constexpr (DevicePixelRatio != 1) {
        image.setDevicePixelRatio(DevicePixelRatio);
    }
    _imgLRUCache.emplace(page, std::move(image));
    update();
}

} // namespace HX