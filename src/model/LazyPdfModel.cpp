#include <model/LazyPdfModel.h>

#include <QtConcurrent>
#include <QPdfDocument>
#include <QPdfPageRenderer>

namespace HX {

LazyPdfModel::LazyPdfModel(int cnt, QPdfDocument* document, QObject* parent)
    : QAbstractListModel(parent)
    , _cnt(cnt)
    , _placeholder(500, 500) 
    , _renderer(new QPdfPageRenderer{this})
{
    _renderer->setDocument(document);
    // 设置为: 所有页面都在单独的工作线程中渲染
    _renderer->setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);

    _placeholder.fill(Qt::darkGray);
    _imageCache.setMaxCost(200);  // 缓存容量
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

    // 开一个新线程, 不阻塞 ui线程
    (void)QtConcurrent::run([this, row]() {
        QString path = QString("E:/图片/图片/neko01.jpg");
        QImage image(path);
        if (image.isNull()) 
            return;
        QImage scaled = image.scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // 跨线程调用 (此处是安全的调用 ui线程 的成员(QAbstractItemModel::dataChanged)信号)
        QMetaObject::invokeMethod(this, [this, row, scaled]() {
            QPixmap* pix = new QPixmap(QPixmap::fromImage(scaled));
            _imageCache.insert(row, pix); // 无需担心析构, QCache会处理好的

            // 触发 UI 更新
            // 告诉 Model 第 row 行有更新, 其内部会调用 data() 进行渲染,
            // 此时 _imageCache.contains(row) 为 true
            emit dataChanged(index(row), index(row), {Qt::DecorationRole});
            
            _pendingLoads.remove(row);
        }, Qt::QueuedConnection /*发送一个QEvent, 并在应用程序进入主事件循环后立即调用*/);
    });
}

} // namespace HX