#include "Preferences.h"

#include "BrewerColorDelegate.h"
#include "BrewerColorSet.h"
#include "BrewerColorTableModel.h"
#include "Pipeline.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QColorDialog>
#include <QtGui/QListView>

Preferences::Preferences(QWidget* parent, Pipeline* pipeline) :
  QDialog(parent), pipeline(pipeline)
{
  ui.setupUi(this);

  connect(ui.backgroundColourButton, SIGNAL(pressed()), this,
      SLOT(setBackgroundColour()));
  connect(ui.bezelColourButton, SIGNAL(pressed()), this,
      SLOT(setBezelColour()));
  connect(ui.vertexColourButton, SIGNAL(pressed()), this,
      SLOT(setVertexColour()));
  connect(ui.edgeColourButton, SIGNAL(pressed()), this, SLOT(setEdgeColour()));
  connect(ui.vertexLabelColourButton, SIGNAL(pressed()), this,
      SLOT(setVertexLabelColour()));
  connect(ui.edgeLabelColourButton, SIGNAL(pressed()), this,
      SLOT(setEdgeLabelColour()));
  connect(ui.landmarkOutlineColourButton, SIGNAL(pressed()), this,
      SLOT(setOutlineColour()));
  connect(ui.landmarkLabelColourButton, SIGNAL(pressed()), this,
      SLOT(setLandmarkLabelColour()));
  connect(ui.selectedVertexColourButton, SIGNAL(pressed()), this,
      SLOT(setSelectedVertexColour()));
  connect(ui.selectedEdgeColourButton, SIGNAL(pressed()), this,
      SLOT(setSelectedEdgeColour()));
  connect(ui.useCurrentCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(setUseCurrentAnnotationColour(int)));

  // initialize button colours
  this->setButtonColor(this->pipeline->backgroundColour(),
      this->ui.backgroundColourButton);
  this->setButtonColor(this->pipeline->bezelColour(),
      this->ui.bezelColourButton);
  this->setButtonColor(this->pipeline->graphVertexColour(),
      this->ui.vertexColourButton);
  this->setButtonColor(this->pipeline->graphVertexLabelColour(),
      this->ui.vertexLabelColourButton);
  this->setButtonColor(this->pipeline->graphEdgeColour(),
      this->ui.edgeColourButton);
  this->setButtonColor(this->pipeline->graphEdgeLabelColour(),
      this->ui.edgeLabelColourButton);
  this->setButtonColor(this->pipeline->landmarkOutlineColour(),
      this->ui.landmarkOutlineColourButton);
  this->setButtonColor(this->pipeline->landmarkLabelColour(),
      this->ui.landmarkLabelColourButton);
  this->setButtonColor(this->pipeline->selectedVertexColour(),
      this->ui.selectedVertexColourButton);
  this->setButtonColor(this->pipeline->selectedEdgeColour(),
      this->ui.selectedEdgeColourButton);
  this->ui.useCurrentCheckBox->setChecked(
      this->pipeline->useCurrentAnnotationColour());

  this->ui.customColourcomboBox->setModel(
      BrewerColorTableModel::fromFile(":/Resources/ColorBrewer.txt"));
  this->ui.customColourcomboBox->setItemDelegate(new BrewerColorDelegate);
  connect(this->ui.customColourcomboBox, SIGNAL(currentIndexChanged(int)), this,
      SLOT(customColourSetChanged(int)));
}

Preferences::~Preferences()
{
  //
}

void Preferences::customColourSetChanged(int index)
{
  for (int i = 0; i < QColorDialog::customCount(); ++i)
  {
    QColorDialog::setCustomColor(i, Qt::white);
  }

  BrewerColorSet colors = qVariantValue<BrewerColorSet>(
      this->ui.customColourcomboBox->itemData(index, Qt::DecorationRole));
  int numColors = qMin(colors.colorCount(), QColorDialog::customCount());
  for (int i = 0; i < numColors; ++i)
  {
    QColorDialog::setCustomColor(i, colors.colors().at(i).rgba());
  }
}

void Preferences::setButtonColor(const QColor& color, QPushButton* button)
{
  if (color.isValid())
  {
    QString s("* { background-color: rgb(");
    QString r(QString::number(color.red()));
    QString g(QString::number(color.green()));
    QString b(QString::number(color.blue()));
    QString style = s % r % "," % g % "," % b % ") }";
    button->setStyleSheet(style);
  }
}

void Preferences::setBackgroundColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->backgroundColour(),
      this, "Select background colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.backgroundColourButton);
  this->pipeline->setBackgroundColour(color);
}

void Preferences::setBezelColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->bezelColour(), this,
      "Select bezel colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.bezelColourButton);
  this->pipeline->setBezelColour(color);
}

void Preferences::setVertexColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->graphVertexColour(),
      this, "Select vertex colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.vertexColourButton);
  this->pipeline->setGraphVertexColour(color);
}

void Preferences::setEdgeColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->graphEdgeColour(), this,
      "Select edge colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.edgeColourButton);
  this->pipeline->setGraphEdgeColour(color);
}

void Preferences::setOutlineColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->landmarkOutlineColour(),
      this, "Select landmark outline colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.landmarkOutlineColourButton);
  this->pipeline->setLandmarkOutlineColour(color);
}

void Preferences::setVertexLabelColour()
{
  QColor color = QColorDialog::getColor(
      this->pipeline->graphVertexLabelColour(), this,
      "Select vertex label colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.vertexLabelColourButton);
  this->pipeline->setGraphVertexLabelColour(color);
}

void Preferences::setEdgeLabelColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->graphEdgeLabelColour(),
      this, "Select edge label colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.edgeLabelColourButton);
  this->pipeline->setGraphEdgeLabelColour(color);
}

void Preferences::setLandmarkLabelColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->landmarkLabelColour(),
      this, "Select landmark label colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.landmarkLabelColourButton);
  this->pipeline->setLandmarkLabelColour(color);
}

void Preferences::setSelectedVertexColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->selectedVertexColour(),
      this, "Select selected vertex colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.selectedVertexColourButton);
  this->pipeline->setSelectedVertexColour(color);
}

void Preferences::setSelectedEdgeColour()
{
  QColor color = QColorDialog::getColor(this->pipeline->selectedEdgeColour(),
      this, "Select selected edge colour",
      QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  this->setButtonColor(color, this->ui.selectedEdgeColourButton);
  this->pipeline->setSelectedEdgeColour(color);
}

void Preferences::setUseCurrentAnnotationColour(int state)
{
  if (state == Qt::Unchecked)
  {
    this->pipeline->setUseCurrentAnnotationColourOff();
  }
  else
  {
    this->pipeline->setUseCurrentAnnotationColourOn();
  }
}
