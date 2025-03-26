#include <delegate/VerticalPdfDelegate.h>

#include <QPainter>

#include <model/LazyPdfModel.h>

namespace HX {

VerticalPdfDelegate::VerticalPdfDelegate(LazyPdfModel* model, QObject* parent)
    : QStyledItemDelegate(parent)
    , _model(model) 
{}

void VerticalPdfDelegate::paint(
    QPainter* painter, 
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    if (pix.isNull())
        return;

    // 直接使用 预缩放的 QPixmap
    QRect targetRect = option.rect;

    // 计算居中位置 (但不再重新缩放)
    if (targetRect.width() > pix.width()) {  
        int xOffset = (targetRect.width() - pix.width()) / 2;
        targetRect.setX(targetRect.x() + xOffset);
        targetRect.setWidth(pix.width());
    } else {
        targetRect.setWidth(pix.width());
    }
    targetRect.setHeight(pix.height());

    // painter->setRenderHint(QPainter::Antialiasing, true);
    // painter->setRenderHint(QPainter::TextAntialiasing, true);

    // 直接绘制
    painter->drawPixmap(targetRect, pix);
}

QSize VerticalPdfDelegate::sizeHint(
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return _model->getSize();
}

} // namespace HX