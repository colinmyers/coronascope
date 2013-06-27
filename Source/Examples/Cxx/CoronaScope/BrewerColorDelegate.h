#ifndef BREWERCOLORDELEGATE_H_
#define BREWERCOLORDELEGATE_H_

#include <QtGui/QStyledItemDelegate>

//! Delegate item for a set of BrewerColors.
/*!
Provides a QStyledItemDelagate interface of a BrewerColorSet, the latter does
the actual painting.
*/
class BrewerColorDelegate: public QStyledItemDelegate
{
  Q_OBJECT
public:
  BrewerColorDelegate(QWidget* parent = 0);
  virtual ~BrewerColorDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem &option,
      const QModelIndex &index) const;
  QSize sizeHint(const QStyleOptionViewItem &option,
      const QModelIndex &index) const;
};

#endif /* BREWERCOLORDELEGATE_H_ */
