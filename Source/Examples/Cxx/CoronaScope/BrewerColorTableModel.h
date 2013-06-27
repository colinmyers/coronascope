#ifndef BREWERCOLORTABLEMODEL_H_
#define BREWERCOLORTABLEMODEL_H_

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtGui/QColor>

#include "BrewerColorSet.h"

//! Table model of a list of BrewerColorSet.
class BrewerColorTableModel: public QAbstractTableModel
{
public:
  BrewerColorTableModel(QObject* parent = 0);
  virtual ~BrewerColorTableModel();

  //! Read a model from a text file.
  static BrewerColorTableModel* fromFile(const QString& fileName);

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  //! Qt::DecorationRole and Qt::DisplayRole return a QVariant<BrewerColorSet>.
  virtual QVariant data(const QModelIndex& index,
      int role = Qt::DisplayRole) const;

  //! No header data is returned.
  QVariant headerData(int section, Qt::Orientation orientation, int role =
      Qt::DisplayRole) const;

private:
  QList<BrewerColorSet> ColorSetList;

};

#endif /* BREWERCOLORTABLEMODEL_H_ */
