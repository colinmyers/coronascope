#include "BrewerColorDelegate.h"
#include "BrewerColorSet.h"

BrewerColorDelegate::BrewerColorDelegate(QWidget* parent) :
  QStyledItemDelegate(parent)
{
  //
}

BrewerColorDelegate::~BrewerColorDelegate()
{
  //
}

void BrewerColorDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyledItemDelegate::paint(painter, option, index);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  if (qVariantCanConvert<BrewerColorSet>(index.data()))
  {
    BrewerColorSet colorSet = qVariantValue<BrewerColorSet>(index.data());
    colorSet.paint(painter, option.rect, option.palette);
  }

  painter->restore();
}

QSize BrewerColorDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
  if (qVariantCanConvert<BrewerColorSet>(index.data()))
  {
    BrewerColorSet colorSet = qVariantValue<BrewerColorSet>(index.data());
    return colorSet.sizeHint();
  }
  else
  {
    return QStyledItemDelegate::sizeHint(option, index);
  }
}
