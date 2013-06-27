#ifndef BREWERCOLORSET_H_
#define BREWERCOLORSET_H_

#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

typedef QList<QColor> QColorList;


//! A set of color definitions from the ColorBrewer collection.
/*!
Represent a set of colors from http://ColorBrewer.org.
*/
class BrewerColorSet
{
public:
  BrewerColorSet();
  //! Construct a set from a label and a list of colors.
  BrewerColorSet(QString& label, QColorList& colors);
  ~BrewerColorSet();

  //! Paint a row of small rectangles into rect, one for each color.
  void paint(QPainter* painter, const QRect& rect, const QPalette& palette) const;
  QSize sizeHint() const;

  //! Get the number of colors in the set.
  int colorCount() const;

  //! Get the user-friendly name of this set.
  QString label() const;

  //! Set the user-friendly name of this set.
  void setLabel(QString& label);

  //! Get a copy of the list of colors.
  QColorList colors() const;

  //! Set the list of colors.
  void setColors(QColorList& colors);

private:
  QString Label;
  QColorList Colors;
  QPolygonF Swatch;
};

Q_DECLARE_METATYPE(BrewerColorSet)

#endif /* BREWERCOLOURSET_H_ */
