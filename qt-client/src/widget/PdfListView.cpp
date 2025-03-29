#include <widget/PdfListView.h>

#include <QWheelEvent>
#include <QScrollBar>
#include <QPropertyAnimation>

namespace HX {

PdfListView::PdfListView(QWidget* parent)
    : QListView(parent)
{
    // 绑定动画
    _barAnimation = new QPropertyAnimation{verticalScrollBar(), "value"};
    _barAnimation->setDuration(200); // 动画持续时间
    _barAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 平滑曲线
}

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
    int numDegrees = event->angleDelta().y() * 2;
    int iCurValue = verticalScrollBar()->value(); // 获取当前滚动条位置

    _barAnimation->setEndValue(iCurValue - numDegrees);
    _barAnimation->start();

    event->accept(); // 防止默认滚动行为
}

} // namespace HX