#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtGui/QDialog>
#include "ui_Preferences.h"

class Pipeline;

//! Preferences dialog for setting colours.
class Preferences: public QDialog
{
  Q_OBJECT

public:
  Preferences(QWidget* parent = 0, Pipeline* pipeline = 0);
  ~Preferences();

private:
  Ui::PreferencesClass ui;
  Pipeline* pipeline;

  //! Change the background colour of a button.
  void setButtonColor(const QColor& color, QPushButton* button);

private slots:
  //! Sets the custom colours of QColorDialog to the selected BrewerColorSet.
  void customColourSetChanged(int index);

  //! Set background colour of QVTK widget window.
  void setBackgroundColour();
  void setBezelColour();
  void setVertexColour();
  void setEdgeColour();
  void setOutlineColour();

  void setVertexLabelColour();
  void setEdgeLabelColour();
  void setLandmarkLabelColour();

  void setSelectedVertexColour();
  void setSelectedEdgeColour();
  void setUseCurrentAnnotationColour(int state);
};

#endif // PREFERENCES_H
