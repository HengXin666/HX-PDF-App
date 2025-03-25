#include <model/LazyPdfModel.h>

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
        qDebug() << "已渲染" << pageNumber;

        // 无需担心析构, QCache会处理好的
        _imageCache.insert(pageNumber, new QPixmap(QPixmap::fromImage(image)));

        // 触发 UI 更新
        // 告诉 Model 第 pageNumber 行有更新, 其内部会调用 data() 进行渲染,
        // 此时 _imageCache.contains(pageNumber) 为 true
        emit dataChanged(index(pageNumber), index(pageNumber), {Qt::DecorationRole});
        
        _pendingLoads.remove(pageNumber);
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
    const_cast<LazyPdfModel*>(this)->loadImage(row);
    return _placeholder;
}

void LazyPdfModel::preloadVisibleArea(int start, int end, int margin) {
    start = qMax(0, start - margin);
    end = qMin(rowCount() - 1, end + margin);
    for (int i = start; i <= end; ++i) {
        if (!_imageCache.contains(i) && !_pendingLoads.contains(i))
            loadImage(i);
    }
}

void LazyPdfModel::loadImage(int row) {
    // 记录正在加载的图片, 防止重复加载
    _pendingLoads.insert(row);

    if (_cnt == 0)
        return;

    auto [w, h] = _baseSize;
    w *= _zoomFactor;
    h *= _zoomFactor;

    // 加入渲染队列
    qDebug() << "渲染中:" << row;
    _renderer->requestPage(row, QSize{static_cast<int>(w), static_cast<int>(h)});
}

} // namespace HX