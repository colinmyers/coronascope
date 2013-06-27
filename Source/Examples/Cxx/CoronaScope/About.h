#ifndef ABOUT_H
#define ABOUT_H

#include <QtGui/QDialog>
#include "ui_About.h"

//! About box.
/*!
A helpful about box with key/mouse bindings.
*/
class About: public QDialog
{
  Q_OBJECT

public:
  About(QWidget* parent = 0);
  ~About();

private:
  Ui::AboutClass ui;
};

#endif // ABOUT_H
