#include "BrewerColorTableModel.h"

#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QColor>

BrewerColorTableModel::BrewerColorTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  //
}

BrewerColorTableModel::~BrewerColorTableModel()
{
  //
}

BrewerColorTableModel* BrewerColorTableModel::fromFile(const QString& fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
  {
    return new BrewerColorTableModel;
  }
#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

  BrewerColorTableModel* m = new BrewerColorTableModel;
  QStringList lines;
  BrewerColorSet colorSet;
  QColorList colorList;
  while (!file.atEnd())
  {
    QByteArray line = file.readLine();
    if (line.isEmpty() || line.at(0) == '#')
    {
      continue;
    }
    lines << line.trimmed();
  }

  QStringList::iterator line;
  for (line = lines.begin(); line != lines.end(); ++line)
  {
    QStringList tokens = line->split(' ');
    if (tokens.size() == 1) // Start of a set
    {
      if (!colorList.isEmpty())
      {
        colorSet.setColors(colorList);
        m->ColorSetList.append(colorSet);
        colorList.clear();
      }
      QString label(tokens.at(0));
      colorSet.setLabel(label);
    }
    else if (tokens.size() == 3) // RGB value
    {
      QColor color(tokens.at(0).toInt(), tokens.at(1).toInt(),
          tokens.at(2).toInt());
      colorList.append(color);
    }
  }

  if (!colorList.isEmpty())
  {
    colorSet.setColors(colorList);
  }
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  return m;
}

int BrewerColorTableModel::rowCount(const QModelIndex& parent) const
{
  return this->ColorSetList.size();
}

int BrewerColorTableModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}

QVariant BrewerColorTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::DecorationRole)
  {
    return qVariantFromValue<BrewerColorSet>(this->ColorSetList.at(index.row()));
  }
  if (role == Qt::DisplayRole)
  {
    return qVariantFromValue<BrewerColorSet>(this->ColorSetList.at(index.row()));
  }
  return QVariant();
}

QVariant BrewerColorTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
  return QVariant();
}
