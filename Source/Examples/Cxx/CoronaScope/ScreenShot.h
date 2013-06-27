#ifndef ScreenShot_H
#define ScreenShot_H

#include "ui_ScreenShot.h"

#include <QtGui/QDialog>
#include <QtGui/QImage>
class Pipeline;

//! Capture the QVTK widget's content and save to a file.
class ScreenShot: public QDialog
{
  Q_OBJECT

public:
  ScreenShot(QWidget* parent = 0, Pipeline* pipeline = 0);
  ~ScreenShot();

public slots:
  //! Capture the current scene - zoom level set via ui.
  void capture();

  //! Save image to selected file - quality set via ui.
  void saveAsDialog();

  private:
  Ui::ScreenShotDialog ui;
  Pipeline* pipeline;
  QImage image;
};

#endif // ScreenShot_H
