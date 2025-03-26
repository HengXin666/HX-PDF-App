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

    // 保持原始比例缩放
    double aspectRatio = static_cast<double>(pix.width()) / pix.height();
    int scaledHeight = _model->getSize().height();
    int scaledWidth = static_cast<int>(aspectRatio * scaledHeight);

    // 计算居中绘制位置
    QRect targetRect = option.rect;
    if (targetRect.width() > scaledWidth) {  // 窗口比图片宽, 居中
        int xOffset = (targetRect.width() - scaledWidth) / 2;
        targetRect.setX(targetRect.x() + xOffset);
        targetRect.setWidth(scaledWidth);
    } else {  // 窗口比图片窄, 等比例缩放
        targetRect.setWidth(scaledWidth);
    }
    targetRect.setHeight(scaledHeight);

    // 白色背景
    painter->fillRect(targetRect, Qt::white);

    // 渲染pdf
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