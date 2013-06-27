#include "ScreenShot.h"

#include "Pipeline.h"
#include <QtCore/QDir>
#include <QtCore/QListIterator>
#include <QtGui/QFileDialog>
#include <QtGui/QImageWriter>

#include <iostream>

ScreenShot::ScreenShot(QWidget *parent, Pipeline* pipeline) :
  QDialog(parent), pipeline(pipeline)
{
  ui.setupUi(this);
  connect(ui.recapturePushButton, SIGNAL(pressed()), this, SLOT(capture()));
  connect(ui.saveAsPushButton, SIGNAL(pressed()), this, SLOT(saveAsDialog()));
  capture();
}

ScreenShot::~ScreenShot()
{
  //
}

void ScreenShot::capture()
{
  image = this->pipeline->windowToImage(ui.zoomSpinBox->value());
  ui.previewLabel->setPixmap(
      QPixmap::fromImage(image).scaledToHeight(ui.previewLabel->height()));
}

void ScreenShot::saveAsDialog()
{
  QString filter;
  QString selectedFilter;
  QListIterator<QByteArray> formats(QImageWriter::supportedImageFormats());
  while (formats.hasNext())
  {
    QString extension(formats.next());
    filter += extension + " files (*." + extension + ");;";
  }

  QString fileName = QFileDialog::getSaveFileName(this, "Save as...",
      QDir::currentPath() + "/screenshot.png", filter, &selectedFilter);
  if (!fileName.isEmpty())
  {
    image.save(fileName, 0, ui.qualitySpinBox->value());
  }
}
