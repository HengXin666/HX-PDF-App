#include <QApplication>
#include <window/MainWindow.h>

int _main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

#include <QApplication>
#include <QLabel>
#include <QPdfDocument>
#include <QPdfPageRenderer>
#include <QImage>
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include <QPdfDocumentRenderOptions>

class PdfViewer : public QWidget {
    Q_OBJECT

public:
    PdfViewer(const QString &filePath, QWidget *parent = nullptr)
        : QWidget(parent), pdfDocument(new QPdfDocument(this)), renderer(new QPdfPageRenderer(this)), label(new QLabel(this)) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(label);
        setLayout(layout);

        // 加载 PDF
        pdfDocument->load(filePath);
        if (pdfDocument->status() != QPdfDocument::Status::Ready) {
            label->setText("加载 PDF 失败！");
            return;
        }

        // 设定渲染器
        renderer->setDocument(pdfDocument);
        renderer->setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);  // 多线程渲染

        // 连接信号，监听渲染完成
        connect(renderer, &QPdfPageRenderer::pageRendered, this, &PdfViewer::onPageRendered);

        // 请求渲染第一页
        requestRenderPage(302);
    }

    QImage InverseColorImage(const QImage &img) {
        QImage imgCopy;
        if (img.format() != QImage::Format_RGB888) {
            imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
        } else {
            imgCopy = QImage(img);
        }
        uint8_t *rgb = imgCopy.bits();
        if (nullptr == rgb) {
            return QImage();
        } int size = img.width() * img.height();
        for (int i = 0; i < size ; i++) {
            rgb[i * 3] = 255 - rgb[i * 3];
            rgb[i * 3 + 1] = 255 - rgb[i * 3 + 1];
            rgb[i * 3 + 2] = 255 - rgb[i * 3 + 2];
        }
        return imgCopy;
    }


private slots:
    void onPageRendered(int pageNumber, QSize imageSize, const QImage &image, QPdfDocumentRenderOptions options, quint64 requestId) {
        Q_UNUSED(pageNumber);
        label->setPixmap(QPixmap::fromImage(
            InverseColorImage(image)
        ));
    }

private:
    void requestRenderPage(int pageNumber) {
        QSize pageSize = pdfDocument->pagePointSize(pageNumber).toSize();
        QPdfDocumentRenderOptions options;
        options.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::Annotations);  // 允许渲染注释
        renderer->requestPage(pageNumber, pageSize, options);
    }

    QPdfDocument *pdfDocument;
    QPdfPageRenderer *renderer;
    QLabel *label;
};

int __main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // PdfViewer viewer("D:/command/编程学习资料/04、GitHub使用帮助.pdf");
    PdfViewer viewer("D:/command/小组/C++-Templates-The-Complete-Guide-zh-20220903.pdf");
    viewer.show();
    return app.exec();
}

#include <QApplication>
#include <QListView>
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QPixmap>
#include <QCache>
#include <QtConcurrent>
#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QWheelEvent>

class LazyImageModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit LazyImageModel(QObject *parent = nullptr)
        : QAbstractListModel(parent), scaleFactor(1.0), baseHeight(500) {
        placeholder.fill(Qt::darkGray);
        imageCache.setMaxCost(200);  // 缓存容量
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return 1000;  // 假设有1000张图片
    }

    QVariant data(const QModelIndex &index, int role = Qt::DecorationRole) const override {
        if (!index.isValid()) return QVariant();

        int row = index.row();
        if (role == Qt::DecorationRole) {
            if (imageCache.contains(row))
                return *imageCache.object(row);

            // 触发异步加载
            const_cast<LazyImageModel*>(this)->loadImage(row);
            return placeholder;
        }
        return QVariant();
    }

    void preloadVisibleArea(int start, int end) {
        int margin = 5;  // 预加载范围
        start = qMax(0, start - margin);
        end = qMin(rowCount() - 1, end + margin);
        for (int i = start; i <= end; ++i) {
            if (!imageCache.contains(i) && !pendingLoads.contains(i))
                loadImage(i);
        }
    }

    void setScaleFactor(double factor) {
        scaleFactor = factor;
        imageCache.clear();  // 缩放时重新加载
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::DecorationRole});
    }

    int getScaledHeight() const {
        return static_cast<int>(baseHeight * scaleFactor);
    }

signals:
    void needUpdate();

private:
    void loadImage(int row) {
        pendingLoads.insert(row);
        double currentScale = scaleFactor;
        QtConcurrent::run([this, row, currentScale]() {
            QString path = QString("E:/图片/图片/neko01.jpg");  // 替换为实际路径
            QImage image(path);
            if (image.isNull()) return;

            QImage scaled = image.scaledToHeight(static_cast<int>(baseHeight * currentScale),
                                                 Qt::SmoothTransformation);
            QMetaObject::invokeMethod(this, [this, row, scaled]() {
                pendingLoads.remove(row);
                QPixmap *pix = new QPixmap(QPixmap::fromImage(scaled));
                imageCache.insert(row, pix);
                emit dataChanged(index(row), index(row), {Qt::DecorationRole});
            }, Qt::QueuedConnection);
        });
    }

    double scaleFactor;
    int baseHeight;
    QPixmap placeholder;
    mutable QCache<int, QPixmap> imageCache;
    QSet<int> pendingLoads;
};

class VerticalImageDelegate : public QStyledItemDelegate {
public:
    explicit VerticalImageDelegate(LazyImageModel *model, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), model(model) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
        painter->drawPixmap(option.rect, pix);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSize(500, model->getScaledHeight());  // 高度随缩放变化
    }

private:
    LazyImageModel *model;
};

class ScalableListView : public QListView {
    Q_OBJECT
public:
    explicit ScalableListView(QWidget *parent = nullptr) : QListView(parent) {}

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) {  // 按 Ctrl 滚动缩放
            double scaleStep = 0.1;
            if (event->angleDelta().y() > 0)
                scaleFactor = qMin(scaleFactor + scaleStep, 3.0);
            else
                scaleFactor = qMax(scaleFactor - scaleStep, 0.5);
            emit scaleChanged(scaleFactor);
            event->accept();
        } else {
            QListView::wheelEvent(event);
        }
    }

signals:
    void scaleChanged(double factor);

private:
    double scaleFactor = 1.0;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ScalableListView listView;
    listView.setUniformItemSizes(true);
    listView.setVerticalScrollMode(QListView::ScrollPerPixel);

    LazyImageModel *model = new LazyImageModel(&listView);
    listView.setModel(model);
    listView.setItemDelegate(new VerticalImageDelegate(model, &listView));

    auto updateVisibleArea = [&]() {
        QRect viewportRect = listView.viewport()->rect();
        int rowHeight = model->getScaledHeight();
        int startRow = listView.verticalScrollBar()->value() / rowHeight;
        int endRow = (listView.verticalScrollBar()->value() + viewportRect.height()) / rowHeight;
        model->preloadVisibleArea(startRow, endRow);
    };

    QObject::connect(listView.verticalScrollBar(), &QScrollBar::valueChanged, updateVisibleArea);
    QObject::connect(&listView, &ScalableListView::scaleChanged, model, &LazyImageModel::setScaleFactor);
    QTimer::singleShot(500, updateVisibleArea);

    listView.show();
    return app.exec();
}

#include "main.moc"
