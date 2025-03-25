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
    painter->drawPixmap(option.rect, pix);
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