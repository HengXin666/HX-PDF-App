#include <widget/MuMainWidget.h>

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QPaintEvent>
#include <QPainter>

#include <mu/Page.h>
#include <mu/PageRenderer.h>

namespace HX {

static constexpr int DPI = 150;
static constexpr int DevicePixelRatio = 1;

QSizeF globalSizeFToLogical(QSizeF globalSize, QScreen* screen = QApplication::primaryScreen()) {
    qreal scaleFactor = screen->devicePixelRatio();
    return QSizeF(globalSize.width() / scaleFactor, globalSize.height() / scaleFactor);
}

QSize globalSizeToLogical(QSize globalSize, QScreen* screen = QApplication::primaryScreen()) {
    qreal scaleFactor = screen->devicePixelRatio();
    return QSize(globalSize.width() / scaleFactor, globalSize.height() / scaleFactor);
}

MuMainWidget::MuMainWidget(QWidget* parent)
    : QWidget(parent)
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
    connect(_pageRenderer, &HX::Mu::PageRenderer::pageReady,
            this, &MuMainWidget::loadPage);
    grabGesture(Qt::SwipeGesture);
    qDebug() << "DPI" << _dpi;
}

void MuMainWidget::setDocument(const QString& filePath) {
    auto tmpByteArr = filePath.toUtf8();
    const char* filePathCharPtr = tmpByteArr.constData();
    _doc = std::make_unique<HX::Mu::Document>(filePathCharPtr);
    try {
        // todo 设置自定义网络流
        _doc->setStream({}).buildDocument(filePathCharPtr);
    } catch (std::exception const& ec) {
        // todo Debug使用的, 正式情况下, 这里不会抛异常!
        qDebug() << ec.what();
        throw ec;
    }

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
    QPainter painter{this};

    // 很卡, 但是是我们需要的!
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

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
            painter.drawImage((width() - size.width()) / 2, y, img);
            emit updatePdfPosInfo(_pageIndex, _totalPages, _zoom);
        } else {
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
    image.setDevicePixelRatio(DevicePixelRatio);
    _imgLRUCache.emplace(page, std::move(image));
    update();
}

} // namespace HX