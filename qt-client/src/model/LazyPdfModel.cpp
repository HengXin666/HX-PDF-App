#include <model/LazyPdfModel.h>

#include <QPainter>
#include <QtConcurrent>
#include <QPdfDocument>
#include <QPdfPageRenderer>

namespace HX {

LazyPdfModel::LazyPdfModel(QPdfDocument* document, QObject* parent)
    : QAbstractListModel(parent)
    , _cnt(document->pageCount())
    , _baseSize(document->pagePointSize(0))
    , _placeholder(_baseSize.toSize()) 
    , _renderer(new QPdfPageRenderer{this})
{
    _renderer->setDocument(document);

    // 设置为: 所有页面都在单独的工作线程中渲染
    _renderer->setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);

    _placeholder.fill(Qt::darkGray);
    _imageCache.setMaxCost(200);  // 缓存容量

    // 渲染完成
    connect(_renderer, &QPdfPageRenderer::pageRendered, this,
        [this](
        int pageNumber,
        QSize imageSize,
        const QImage& image,
        QPdfDocumentRenderOptions options,
        quint64 requestId
    ) {
        (void)QtConcurrent::run([this, pageNumber, image]() {
            QImage finalImage(image.size(), QImage::Format_ARGB32_Premultiplied);
            finalImage.fill(Qt::white); // 先填充背景
            QPainter painter(&finalImage);
            painter.setRenderHint(QPainter::TextAntialiasing, true); // 启用抗锯齿
            painter.drawImage(0, 0, image);
            painter.end();

            auto* pixmap = new QPixmap(QPixmap::fromImage(finalImage.scaled(
                finalImage.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation))
            );

            QMetaObject::invokeMethod(this,
                [this, pageNumber, pixmap]() {
                // 无需担心析构, QCache会处理好的
                _imageCache.insert(pageNumber, pixmap);
                // 触发 UI 更新
                // 告诉 Model 第 pageNumber 行有更新, 其内部会调用 data() 进行渲染,
                // 此时 _imageCache.contains(pageNumber) 为 true
                emit dataChanged(index(pageNumber), index(pageNumber), {Qt::DecorationRole});
                _pendingLoads.remove(pageNumber);
            });
        });
    });

    // 页数变化 todo !!!以后改为根据状态!!!
    connect(document, &QPdfDocument::pageCountChanged, this,
        [this, document](int pageCount) {
        qDebug() << "新加载的页数:" << pageCount;
        _cnt = pageCount;
        _baseSize = document->pagePointSize(0);
    });
}

QVariant LazyPdfModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DecorationRole) 
        return QVariant{};

    int row = index.row();
    if (_imageCache.contains(row))
        return *_imageCache.object(row);
    
    // 异步加载图片
    // 优化: 仅当行索引在可视范围+一定预加载范围内, 才触发 loadImage()
    if (_visibleRange.l <= row && row <= _visibleRange.r) {
        const_cast<LazyPdfModel*>(this)->loadImage(row);
    }
    return _placeholder;
}

void LazyPdfModel::preloadVisibleArea(int start, int end, int margin) {
    start = qMax(0, start - margin);
    end = qMin(rowCount() - 1, end + margin);
    for (int i = start; i <= end; ++i) {
        if (!_imageCache.contains(i)) {
            loadImage(i);
        }
    }
}

void LazyPdfModel::loadImage(int row) {
    if (_cnt == 0 || _pendingLoads.contains(row))
        return;

    // 记录正在加载的图片, 防止重复加载
    _pendingLoads.insert(row);

    auto [w, h] = _baseSize;
    w *= _zoomFactor;
    h *= _zoomFactor;

    // 加入渲染队列
    QPdfDocumentRenderOptions opt;

    QSize originalSize = {static_cast<int>(w), static_cast<int>(h)};  // 获取 PDF 页面的原始尺寸（像素）
    // QSize highDpiSize = originalSize * (300.0 / 72.0);     // 按 300 DPI 计算目标尺寸

    // opt.setScaledSize(highDpiSize);
    opt.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::Annotations); // 渲染 PDF 注释
    
    _renderer->requestPage(
        row,
        originalSize,
        opt
    );
}

} // namespace HX