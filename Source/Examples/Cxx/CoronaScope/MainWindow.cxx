#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "About.h"
#include "Pipeline.h"
#include "Preferences.h"
#include "ScreenShot.h"

#include "vtkAnnotatedGraphView.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLink.h"
#include "vtkCommand.h"
#include "vtkDataRepresentation.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkInformation.h"
#include "vtkMyQtAnnotationLayersModelAdapter.h"
#include "vtkObject.h"
#include "vtkRenderWindow.h"

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QKeyEvent>

MainWindow::MainWindow()
{
  this->ui.setupUi(this);
  this->eventConnector = 0;
  this->annotationModel = 0;

  QCoreApplication::setOrganizationName("University of Leeds");
  QCoreApplication::setOrganizationDomain("leeds.ac.uk");
  QCoreApplication::setApplicationName("CoronaScope");
  this->setWindowTitle(QCoreApplication::applicationName());
  this->setWindowIcon(QIcon(":/Resources/corona.xpm"));

  this->pipeline = new Pipeline;
  this->pipeline->setInteractor(this->ui.qvtkWidget->GetInteractor());
  this->ui.qvtkWidget->SetRenderWindow(this->pipeline->renderWindow());
  this->readSettings();

  this->ui.controlPanelWidget->setEnabled(false);
  this->annotationEnabledState = true;

  connect(this->ui.actionScreenShot, SIGNAL(triggered()), this,
      SLOT(openScreenShotDialog()));
  connect(this->ui.actionControlPanel, SIGNAL(toggled(bool)),
      this->ui.dockWidget, SLOT(setVisible(bool)));
  connect(this->ui.dockWidget, SIGNAL(visibilityChanged(bool)),
      this->ui.actionControlPanel, SLOT(setChecked(bool)));
  connect(this->ui.actionFullScreen, SIGNAL(toggled(bool)), this,
      SLOT(fullScreenToggle(bool)));

  connect(this->ui.actionAboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()));
  connect(this->ui.actionAboutCoRoNaScope, SIGNAL(triggered()), this,
      SLOT(about()));
  connect(this->ui.actionExit, SIGNAL(triggered()), this, SLOT(closeWindow()));
  connect(this->ui.actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));
  connect(this->ui.actionOpenFile, SIGNAL(triggered()), this,
      SLOT(openFileDialog()));
  connect(this->ui.actionPreferences, SIGNAL(triggered()), this,
      SLOT(openPreferencesDialog()));

  connect(this->ui.vertexLabelArrayNameComboBox,
      SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(vertexLabelArrayName(const QString&)));
  connect(this->ui.edgeLabelArrayNameComboBox,
      SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(edgeLabelArrayName(const QString&)));

  connect(this->ui.graphLayoutComboBox,
      SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(graphLayout(const QString&)));

  // Add list of layout algorithms
  QStringList vtkLayouts;
  vtkLayouts << "vtkRandom"
      << "vtkCircular"
      << "vtkClustering2D"
      << "vtkConeTree"
      << "vtkForceDirected"
      << "vtkSimpleTree";
  this->ui.graphLayoutComboBox->addItems(vtkLayouts);

#ifdef __USE_OGDF__
  this->ui.graphLayoutComboBox->insertSeparator(6);
  QStringList ogdfLayouts;
  ogdfLayouts << "Balloon"
      << "Circular"
      << "Davidson Harel"
      << "FMMM"
      << "GEM"
      << "MMM Example Fast"
      << "MMM Example Nice"
      << "MMM Example No Twist"
      << "Fruchterman Reingold"
      << "Kamada Kawai"
      << "Sugiyama";
  this->ui.graphLayoutComboBox->addItems(ogdfLayouts);
#endif

  connect(this->ui.landmarkLabelCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(landmarkLabelToggle(int)));
  connect(this->ui.landmarkScaleSlider, SIGNAL(valueChanged(int)), this,
      SLOT(landmarkOutputScaling(int)));
  connect(this->ui.minSizeSlider, SIGNAL(valueChanged(int)), this,
      SLOT(landmarkMinimumSize(int)));
  connect(this->ui.outlineCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(landmarkOutlineToggle(int)));
  connect(this->ui.landmarkShapeComboBox,
      SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(landmarkShape(const QString&)));
  connect(this->ui.flightTypeComboBox,
      SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(flightType(const QString&)));
  connect(this->ui.flightSpeedSlider, SIGNAL(valueChanged(int)), this,
      SLOT(flightSpeed(int)));

  connect(this->ui.ospOnOffCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(offScreenProjectionToggle(int)));
  connect(this->ui.ospAutoHideCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(offScreenWidgetAutoDimToggle(int)));
  connect(this->ui.reduceOverlapsCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(overlapReductionToggle(int)));
  connect(this->ui.showErrorCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(showErrorToggle(int)));

  connect(this->ui.newAnnotationButton, SIGNAL(pressed()), this,
      SLOT(newAnnotation()));
  connect(this->ui.deleteAnnotationButton, SIGNAL(pressed()), this,
      SLOT(removeAnnotation()));
  connect(this->ui.selectNoneButton, SIGNAL(pressed()), this,
      SLOT(selectNone()));
  connect(this->ui.disableAllButton, SIGNAL(pressed()), this,
      SLOT(disableAll()));
  connect(this->ui.importAnnotationsButton, SIGNAL(pressed()), this,
      SLOT(loadAnnotationsDialog()));
  connect(this->ui.exportAnnotationsButton, SIGNAL(pressed()), this,
      SLOT(saveAnnotationsDialog()));

  connect(this->ui.annotationTable, SIGNAL(clicked(const QModelIndex&)), this,
      SLOT(selectedAnnotationChanged(const QModelIndex&)));
  connect(this->ui.annotationTable, SIGNAL(doubleClicked(const QModelIndex&)),
      this, SLOT(getAnnotationColourFromUser(const QModelIndex&)));
}

MainWindow::~MainWindow()
{
  delete this->pipeline;
  if (this->eventConnector)
  {
    this->eventConnector->Delete();
  }
}

void MainWindow::showBusyWaitMessage(const QString& message)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  this->statusBar()->showMessage(message);
  QApplication::processEvents();
}

void MainWindow::clearBusyWaitMessage()
{
  QApplication::restoreOverrideCursor();
  this->statusBar()->clearMessage();
  QApplication::processEvents();
}

void MainWindow::newAnnotation()
{
  QModelIndex selected =
      this->ui.annotationTable->selectionModel()->currentIndex();
  if (selected.isValid())
  {
    QModelIndex newIndex(
        this->ui.annotationTable->model()->index(selected.row() + 1, 0));
    if (this->ui.annotationTable->model()->insertRow(newIndex.row(),
        QModelIndex()))
    {
      this->ui.annotationTable->selectRow(newIndex.row());
      this->selectedAnnotationChanged(newIndex);
    }
  }
}

void MainWindow::removeAnnotation()
{
  QModelIndex selected =
      this->ui.annotationTable->selectionModel()->currentIndex();
  if (selected.isValid())
  {
    if (this->ui.annotationTable->model()->removeRow(selected.row(),
        QModelIndex()))
    {
      QModelIndex oldIndex(this->ui.annotationTable->model()->index(0, 0));
      if (oldIndex.isValid())
      {
        this->ui.annotationTable->selectRow(oldIndex.row());
        this->selectedAnnotationChanged(oldIndex);
      }
    }
  }
}

void MainWindow::selectNone()
{
  this->ui.annotationTable->selectionModel()->clearSelection();
  this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers()->SetCurrentAnnotation(
      0);
  this->pipeline->view()->Render();
}

void MainWindow::disableAll()
{
  this->annotationEnabledState = !this->annotationEnabledState;
  vtkAnnotationLayers* annLayers =
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers();
  unsigned int numLayers = annLayers->GetNumberOfAnnotations();
  for (unsigned int i = 0; i < numLayers; ++i)
  {
    vtkAnnotation* ann = annLayers->GetAnnotation(i);
    ann->GetInformation()->Set(vtkAnnotation::ENABLE(), annotationEnabledState);
  }
  annLayers->Modified();
  this->pipeline->view()->Render();
  if (annotationEnabledState)
  {
    this->ui.disableAllButton->setText("Disable &All");
  }
  else
  {
    this->ui.disableAllButton->setText("Enable &All");
  }
}

void MainWindow::selectedAnnotationChanged(const QModelIndex& index)
{
  vtkAnnotation* newCurrentAnnotation =
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers()->GetAnnotation(
          index.row());
  this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers()->SetCurrentAnnotation(
      newCurrentAnnotation);
  this->pipeline->view()->Render();
}

void MainWindow::annotationDataChanged(const QModelIndex&, const QModelIndex&)
{
  vtkAnnotationLink* al =
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink();
  al->Modified();
  this->pipeline->view()->Render();
}

void MainWindow::annotationChanged(vtkObject*, unsigned long, void*, void*)
{
  QModelIndex selected =
      this->ui.annotationTable->selectionModel()->currentIndex();
  this->annotationModel->SetVTKDataObject(0);
  this->annotationModel->SetVTKDataObject(
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers());
  this->ui.annotationTable->selectRow(selected.row());
}

void MainWindow::getAnnotationColourFromUser(const QModelIndex& index)
{
  if (index.column() == 0)
  {
    QColor initial = QColor(
        this->annotationModel->data(index, Qt::DecorationRole).value<QColor>());
    QColor color;
    if (initial.isValid())
    {
      color = QColorDialog::getColor(initial, this, "Select landmark colour",
          QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    }
    else
    {
      initial = Qt::gray;
      color = QColorDialog::getColor(initial, this, "Select landmark colour",
          QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    }
    if (color.isValid())
    {
      this->annotationModel->setData(index, color, Qt::EditRole);
    }
  }
}

void MainWindow::readSettings()
{
  QSettings settings("University of Leeds", "CoronaScope");
  settings.beginGroup("MainWindow");
  QDir::setCurrent(settings.value("dataDir", QDir::currentPath()).toString());
  this->restoreGeometry(settings.value("geometry").toByteArray());
  this->restoreState(settings.value("state").toByteArray());
  settings.endGroup();

  settings.beginGroup("Preferences");
  this->pipeline->setBackgroundColour(
      settings.value("backgroundColour", Qt::white).value<QColor>());
  this->pipeline->setBezelColour(
      settings.value("bezelColour", Qt::black).value<QColor>());
  this->pipeline->setGraphVertexColour(
      settings.value("vertexColour", Qt::gray).value<QColor>());
  this->pipeline->setGraphEdgeColour(
      settings.value("graphEdgeColour", Qt::gray).value<QColor>());
  this->pipeline->setLandmarkOutlineColour(
      settings.value("landmarkOutlineColour", Qt::black).value<QColor>());
  this->pipeline->setGraphVertexLabelColour(
      settings.value("graphVertexLabelColour", Qt::black).value<QColor>());
  this->pipeline->setGraphEdgeLabelColour(
      settings.value("graphEdgeLabelColour", Qt::black).value<QColor>());
  this->pipeline->setLandmarkLabelColour(
      settings.value("landmarkLabelColour", Qt::gray).value<QColor>());
  settings.endGroup();
}

void MainWindow::writeSettings()
{
  QSettings settings("University of Leeds", "CoronaScope");
  settings.beginGroup("MainWindow");
  settings.setValue("dataDir", QDir::currentPath());
  settings.setValue("geometry", this->saveGeometry());
  settings.setValue("state", this->saveState());
  settings.endGroup();

  settings.beginGroup("Preferences");
  settings.setValue("backgroundColour", this->pipeline->backgroundColour());
  settings.setValue("bezelColour", this->pipeline->bezelColour());
  settings.setValue("graphVertexColour", this->pipeline->graphVertexColour());
  settings.setValue("graphEdgeColour", this->pipeline->graphEdgeColour());
  settings.setValue("landmarkOutlineColour",
      this->pipeline->landmarkOutlineColour());
  settings.setValue("graphVertexLabelColour",
      this->pipeline->graphVertexLabelColour());
  settings.setValue("graphEdgeLabelColour",
      this->pipeline->graphEdgeLabelColour());
  settings.setValue("landmarkLabelColour",
      this->pipeline->landmarkLabelColour());
  settings.setValue("useCurrentAnnotationColour",
      this->pipeline->useCurrentAnnotationColour());
  settings.endGroup();
}

void MainWindow::aboutQt()
{
  QApplication::aboutQt();
}

void MainWindow::about()
{
  About dialog(this);
  dialog.exec();
}

void MainWindow::fullScreenToggle(bool b)
{
  if (b)
  {
    this->showFullScreen();
  }
  else
  {
    this->showNormal();
  }
}

void MainWindow::closeWindow()
{
  this->close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  this->writeSettings();
  QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
  this->ui.qvtkWidget->update();
}

void MainWindow::saveAnnotationsDialog()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Export Landmarks",
      QDir::currentPath(), "XML files (*.xml)");
  if (!fileName.isEmpty())
  {
    this->showBusyWaitMessage("Exporting " + fileName);
    this->pipeline->saveAnnotationsToFile(fileName.toStdString().c_str());
    this->clearBusyWaitMessage();
  }
}

void MainWindow::loadAnnotationsDialog()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Import Landmarks",
      QDir::currentPath(), "XML files (*.xml)");
  if (!fileName.isEmpty())
  {
    this->showBusyWaitMessage("Importing " + fileName);
    this->openAnnotationsFile(fileName);
    this->clearBusyWaitMessage();
  }
}

void MainWindow::openAnnotationsFile(const QString& fileName)
{
  this->pipeline->loadAnnotationsFromFile(fileName.toStdString().c_str());
  this->annotationModel->SetVTKDataObject(0);
  this->annotationModel->SetVTKDataObject(
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink()->GetAnnotationLayers());

  this->ui.annotationTable->update();
  this->selectNone();
}

void MainWindow::openFileDialog()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Open Tulip File",
      QDir::currentPath(), "Tulip files (*.tlp)");
  if (!fileName.isEmpty())
  {
    this->showBusyWaitMessage("Opening " + fileName);
    this->openFile(fileName);
    this->clearBusyWaitMessage();
  }
}

void MainWindow::openFile(const QString& fileName)
{
  // set up the pipeline with the new filename
  this->pipeline->setFileName(fileName.toStdString().c_str());

  // update graph label array names
  this->ui.vertexLabelArrayNameComboBox->clear();
  this->ui.edgeLabelArrayNameComboBox->clear();
  this->ui.vertexLabelArrayNameComboBox->addItems(
      this->pipeline->graphVertexLabelArrayNames());
  this->ui.edgeLabelArrayNameComboBox->addItems(
      this->pipeline->graphEdgeLabelArrayNames());

  // connect annotation change events between Qt and VTK
  if (this->eventConnector)
  {
    this->eventConnector->Delete();
  }
  this->eventConnector = vtkEventQtSlotConnect::New();
  this->eventConnector->Connect(this->pipeline->view()->GetRepresentation(),
      vtkCommand::AnnotationChangedEvent, this,
      SLOT(annotationChanged(vtkObject*, unsigned long, void*, void*)));

  this->annotationModel = new vtkMyQtAnnotationLayersModelAdapter(this);
  this->ui.annotationTable->setModel(this->annotationModel);
  this->annotationModel->SetVTKDataObject(
      this->pipeline->view()->GetRepresentation()->GetAnnotationLink()
      ->GetAnnotationLayers());

  connect(this->annotationModel,
      SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this,
      SLOT(annotationDataChanged(const QModelIndex&, const QModelIndex&)));
  this->selectNone();

  // update the ui
  this->ui.qvtkWidget->update();
  this->ui.controlPanelWidget->setEnabled(true);
  this->ui.controlPanelWidget->setCurrentIndex(0);
  if (this->ui.ospOnOffCheckBox->isChecked())
  {
    this->pipeline->setOffScreenProjectionOn();
  }

  QFileInfo fileInfo(fileName);
  QDir::setCurrent(fileInfo.absolutePath());
  this->setWindowTitle(
      QCoreApplication::applicationName() + " - " + fileName);
  this->ui.annotationTable->resizeColumnsToContents();
}

void MainWindow::closeFile()
{
  this->setWindowTitle(QCoreApplication::applicationName());
  this->ui.controlPanelWidget->setEnabled(false);
  this->pipeline->setFileName(0);
}

void MainWindow::openPreferencesDialog()
{
  Preferences p(this, this->pipeline);
  p.exec();
}

void MainWindow::openScreenShotDialog()
{
  ScreenShot s(this, this->pipeline);
  s.exec();
}

void MainWindow::graphLayout(const QString& algorithmName)
{
  this->showBusyWaitMessage(algorithmName + " layout...");

  if (algorithmName.compare("vtkCircular") == 0)
  {
    this->pipeline->setLayoutToCircular();
  }
  else if (algorithmName.compare("vtkForceDirected") == 0)
  {
    this->pipeline->setLayoutToForceDirected();
  }
  else if (algorithmName.compare("vtkSimpleTree") == 0)
  {
    this->pipeline->setLayoutToSimpleTree();
  }
  else if (algorithmName.compare("vtkConeTree") == 0)
  {
    this->pipeline->setLayoutToCone();
  }
  else if (algorithmName.compare("vtkClustering2D") == 0)
  {
    this->pipeline->setLayoutToClustering2D();
  }
  else if (algorithmName.compare("vtkRandom") == 0)
  {
    this->pipeline->setLayoutToRandom();
  }
  else
  {
    this->pipeline->setLayoutToOGDF(algorithmName.toStdString().c_str());
  }
  this->clearBusyWaitMessage();
  this->ui.qvtkWidget->update();
}

void MainWindow::vertexLabelArrayName(const QString& text)
{
  this->pipeline->setGraphVertexLabelArrayName(text);
  this->ui.qvtkWidget->update();
}

void MainWindow::edgeLabelArrayName(const QString& text)
{
  this->pipeline->setGraphEdgeLabelArrayName(text);
  this->ui.qvtkWidget->update();
}

void MainWindow::landmarkLabelToggle(int state)
{
  if (state)
  {
    this->pipeline->setLandmarkLabelsOn();
  }
  else
  {
    this->pipeline->setLandmarkLabelsOff();
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::landmarkOutputScaling(int value)
{
  this->pipeline->setLandmarkScaling(value);
  this->ui.qvtkWidget->update();
}

void MainWindow::landmarkMinimumSize(int value)
{
  this->pipeline->setMinimumSize(value);
  this->ui.qvtkWidget->update();
}

void MainWindow::landmarkOutlineToggle(int state)
{
  if (state)
  {
    this->pipeline->setLandmarkOutlineOn(true);
  }
  else
  {
    this->pipeline->setLandmarkOutlineOn(false);
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::landmarkShape(const QString& text)
{
  if (text.compare("Convex hull") == 0)
  {
    this->pipeline->setLandmarkToConvexHull();
  }
  else if (text.compare("Rectangle") == 0)
  {
    this->pipeline->setLandmarkToRectangle();
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::flightType(const QString& text)
{
  if (text.compare("Express") == 0)
  {
    this->pipeline->setFlightTypeToExpress();
  }
  else if (text.compare("Tourist") == 0)
  {
    this->pipeline->setFlightTypeToTourist();
  }
}

void MainWindow::flightSpeed(int value)
{
  this->pipeline->setFlightSpeedBias(-value);
}

void MainWindow::offScreenProjectionToggle(int state)
{
  if (state)
  {
    this->pipeline->setOffScreenProjectionOn();
  }
  else
  {
    this->pipeline->setOffScreenProjectionOff();
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::offScreenWidgetAutoDimToggle(int state)
{
  if (state)
  {
    this->pipeline->setWidgetAutoDimOn();
  }
  else
  {
    this->pipeline->setWidgetAutoDimOff();
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::overlapReductionToggle(int state)
{
  if (state)
  {
    this->pipeline->setOverlapReductionOn();
  }
  else
  {
    this->pipeline->setOverlapReductionOff();
  }
  this->ui.qvtkWidget->update();
}

void MainWindow::showErrorToggle(int state)
{
  if (state)
  {
    this->pipeline->setShowErrorOn();
  }
  else
  {
    this->pipeline->setShowErrorOff();
  }
  this->ui.qvtkWidget->update();
}
