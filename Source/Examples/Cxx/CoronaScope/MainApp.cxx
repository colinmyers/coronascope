#include <QtGui/QApplication>
#include "MainWindow.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  MainWindow win;

  if (argc > 1)
  {
    QString fileName(argv[1]);
    win.openFile(fileName);
  }

  if (argc > 2)
  {
    QString annotationFileName(argv[2]);
    win.openAnnotationsFile(annotationFileName);
  }

  if (argc > 3)
  {
    QString graphLayout(argv[3]);
    win.graphLayout(graphLayout);
  }

  win.show();

  return app.exec();
}

