#include <widget/PdfListView.h>

#include <QWheelEvent>

namespace HX {

PdfListView::PdfListView(QWidget* parent)
    : QListView(parent)
{}

int PdfListView::getVisibleIndex() const {
    auto* md = model();
    int rowCount = md->rowCount();

    // 获取视口的可见区域
    QRect viewportRect = viewport()->rect();

    // 获取第一个可见索引
    QModelIndex firstIndex = indexAt(viewportRect.topLeft());
    return firstIndex.isValid() ? firstIndex.row() : 0;
}

void PdfListView::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {  // 按 Ctrl 滚动缩放
        double scaleStep = 0.1;
        if (event->angleDelta().y() > 0)
            _zoomFactor = qMin(_zoomFactor + scaleStep, 3.0);
        else
            _zoomFactor = qMax(_zoomFactor - scaleStep, 0.5);
        emit scaleChanged(_zoomFactor);
        event->accept();
    } else {
        QListView::wheelEvent(event);
    }
}

} // namespace HX