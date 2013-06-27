#ifndef MainWindow_H
#define MainWindow_H

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"

class Ui_MainWindow;
class Pipeline;

class vtkEventQtSlotConnect;
class vtkMyQtAnnotationLayersModelAdapter;
class vtkObject;

class QCloseEvent;
class QItemSelection;
class QKeyEvent;
class QModelIndex;
class QResizeEvent;
class QString;

//! The main application window of CoronaScope.
/*!
  Most slots in this class call methods on the Pipeline object. In the case of
  dialogs, they are passed a pointer to the Pipeline.
 */
class MainWindow: public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  virtual ~MainWindow();

  //! Open a Tulip (*.tlp) file.
  void openFile(const QString& fileName);

  //! Open a file containing serialized vtk annotations (*.xml).
  void openAnnotationsFile(const QString& fileName);

public slots:
  //! Run the graph layout algorithm.
  void graphLayout(const QString& algorithmName);

protected:
  //! Spin the cursor and display message in the status bar.
  void showBusyWaitMessage(const QString& message);

  //! Restore the cursor and clear the status bar.
  void clearBusyWaitMessage();

  //! Read config from file/registry. Called on entry.
  void readSettings();

  //! Write config to file/registry. Called on closeEvent.
  void writeSettings();

  //! Write the settings.
  virtual void closeEvent(QCloseEvent* event);

  //! Update the QVTK widget following a resize.
  virtual void resizeEvent(QResizeEvent* event);

protected slots:
  // Annotations model
  void newAnnotation();
  void removeAnnotation();
  void selectNone();
  void disableAll();
  void annotationChanged(vtkObject*, unsigned long, void*, void*);
  void annotationDataChanged(const QModelIndex&, const QModelIndex&);
  void selectedAnnotationChanged(const QModelIndex& index);
  void getAnnotationColourFromUser(const QModelIndex& index);

  // Dialogs
  void aboutQt();
  void about();
  void saveAnnotationsDialog();
  void loadAnnotationsDialog();
  void openFileDialog();
  void openPreferencesDialog();
  void openScreenShotDialog();

  // Misc
  void fullScreenToggle(bool b);
  void closeWindow();
  void closeFile();

  // Pipeline modifiers
  void vertexLabelArrayName(const QString& text);
  void edgeLabelArrayName(const QString& text);

  void offScreenProjectionToggle(int state);
  void offScreenWidgetAutoDimToggle(int state);
  void overlapReductionToggle(int state);
  void showErrorToggle(int state);

  void landmarkLabelToggle(int state);
  void landmarkOutputScaling(int value);
  void landmarkMinimumSize(int value);
  void landmarkOutlineToggle(int state);
  void landmarkShape(const QString& text);
  void flightType(const QString& text);
  void flightSpeed(int value);

private:
  Ui_MainWindow ui;
  Pipeline* pipeline;
  vtkEventQtSlotConnect* eventConnector;
  vtkMyQtAnnotationLayersModelAdapter* annotationModel;
  bool annotationEnabledState;
};

#endif // MainWindow_H
