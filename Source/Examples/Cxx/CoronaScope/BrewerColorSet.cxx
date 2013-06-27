#include "BrewerColorSet.h"

#include <QtCore/QPointF>
#include <QtGui/QBrush>
#include <QtGui/QPalette>

BrewerColorSet::BrewerColorSet() :
Label(QString("")), Colors(QColorList())
{
  this->Swatch << QPointF(0.0, 0.1) << QPointF(1.0, 0.1) << QPointF(1.0, 0.9)
          << QPointF(0.0, 0.9) << QPointF(0.0, 0.0);
}

BrewerColorSet::BrewerColorSet(QString& label, QColorList& colors) :
        Label(label), Colors(colors)
{
  this->Swatch << QPointF(0.0, 0.1) << QPointF(1.0, 0.1) << QPointF(1.0, 0.9)
          << QPointF(0.0, 0.9) << QPointF(0.0, 0.0);
}

BrewerColorSet::~BrewerColorSet()
{
  //
}

void BrewerColorSet::paint(QPainter *painter, const QRect &rect,
    const QPalette &palette) const
{
  painter->save();

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);

  int yOffset = (rect.height() - 20) / 2;
  painter->translate(rect.x(), rect.y() + yOffset);
  painter->scale(20, 20);
  painter->setBrush(palette.foreground());

  QColorList::const_iterator color;
  for (color = this->Colors.begin(); color != this->Colors.end(); ++color)
  {
    QBrush brush(*color);
    painter->setBrush(brush);
    painter->drawPolygon(this->Swatch, Qt::WindingFill);
    painter->translate(1.0, 0.0);
  }

  painter->restore();
}

QSize BrewerColorSet::sizeHint() const
{
  return 20 * QSize(4, 1);
}

int BrewerColorSet::colorCount() const
{
  return this->Colors.size();
}

QString BrewerColorSet::label() const
{
  return this->Label;
}

QColorList BrewerColorSet::colors() const
{
  return this->Colors;
}

void BrewerColorSet::setLabel(QString& label)
{
  this->Label = label;
}

void BrewerColorSet::setColors(QColorList& colors)
{
  this->Colors = colors;
}
