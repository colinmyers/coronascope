#include "Pipeline.h"

#include "vtkAnnotatedGraphView.h"
#include "vtkAnnotatedGraphRepresentation.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLayersSerializer.h"
#include "vtkAnnotationLink.h"
#include "vtkFlightMapFilter.h"
#include "vtkGraphLayout.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkOffScreenWidget.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"
#include "vtkTulipReader.h"
#include "vtkWindowToImageFilter.h"

#include "vtkCircularLayoutStrategy.h"
#include "vtkClustering2DLayoutStrategy.h"
#include "vtkConeLayoutStrategy.h"
#include "vtkForceDirectedLayoutStrategy.h"
#include "vtkMySpanTreeLayoutStrategy.h"
#ifdef __USE_OGDF__
#include "vtkOGDFLayoutStrategy.h"
#endif
#include "vtkRandomLayoutStrategy.h"
#include "vtkTreeLayoutStrategy.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QColor>
#include <QtGui/QImage>

#include <istream>
#include <ostream>
#include <sstream>
#include <string>


Pipeline::Pipeline()
{
  this->Reader = vtkSmartPointer<vtkTulipReader>::New();
  this->Layout = vtkSmartPointer<vtkGraphLayout>::New();
  this->Representation =
      vtkSmartPointer<vtkAnnotatedGraphRepresentation>::New();
  this->View = vtkSmartPointer<vtkAnnotatedGraphView>::New();
  this->Widget = vtkSmartPointer<vtkOffScreenWidget>::New();
  this->FlightMap = vtkSmartPointer<vtkFlightMapFilter>::New();

  this->Layout->SetInputConnection(this->Reader->GetOutputPort());
  this->Representation->SetInputConnection(this->Layout->GetOutputPort());
  this->FlightMap->SetInputConnection(0, this->Layout->GetOutputPort());
  this->FlightMap->SetInputConnection(1, this->Reader->GetOutputPort(1));
  this->Widget->SetLandmarkCentres(this->Representation->GetLandmarkCentres());
  this->Widget->SetFlightMap(this->FlightMap);
  this->Widget->SetInteractor(this->View->GetInteractor());

  this->setLayoutToRandom();
  this->View->SetLayoutStrategyToPassThrough();
  this->setOffScreenProjectionOff();
  this->setWidgetAutoDimOff();
  this->setLandmarkScaling(100);
  this->setMinimumSize(0);
  this->setLandmarkOutlineOn(false);
  this->setLandmarkToConvexHull();
  this->setFlightTypeToExpress();
  this->setGraphVertexLabelsOff();
  this->setGraphEdgeLabelsOff();
  this->setLandmarkLabelsOff();

  vtkTextProperty* vertexLabelProperty =
      this->Representation->GetVertexLabelTextProperty();
  vertexLabelProperty->SetFontSize(12);
  vertexLabelProperty->SetBold(1);
  vertexLabelProperty->SetItalic(0);
  vertexLabelProperty->SetShadow(0);
  vertexLabelProperty->SetFontFamilyToArial();
  vertexLabelProperty->SetJustificationToCentered();
  vertexLabelProperty->SetVerticalJustificationToCentered();
  vertexLabelProperty->SetLineOffset(10.0);

  vtkTextProperty* edgeLabelProperty =
      this->Representation->GetEdgeLabelTextProperty();
  edgeLabelProperty->SetFontSize(10);
  edgeLabelProperty->SetBold(1);
  edgeLabelProperty->SetItalic(0);
  edgeLabelProperty->SetShadow(0);
  edgeLabelProperty->SetFontFamilyToArial();
  edgeLabelProperty->SetJustificationToCentered();
  edgeLabelProperty->SetVerticalJustificationToCentered();

  vtkTextProperty* landmarkLabelProperty =
      this->Representation->GetLandmarkLabelTextProperty();
  landmarkLabelProperty->SetFontSize(18);
  landmarkLabelProperty->SetBold(1);
  landmarkLabelProperty->SetItalic(0);
  landmarkLabelProperty->SetShadow(0);
  landmarkLabelProperty->SetFontFamilyToArial();
  landmarkLabelProperty->SetJustificationToCentered();
  landmarkLabelProperty->SetVerticalJustificationToCentered();
}

Pipeline::~Pipeline()
{
  //
}

vtkAnnotatedGraphView* Pipeline::view() const
{
  return this->View;
}

vtkRenderWindow* Pipeline::renderWindow() const
{
  return this->View->GetRenderWindow();
}

void Pipeline::setInteractor(vtkRenderWindowInteractor* iren)
{
  this->View->SetInteractor(iren);
}

void Pipeline::setFileName(const char* fileName)
{
  this->Reader->SetFileName(fileName);
  if (fileName)
  {
    this->connect();
  }
  else
  {
    this->disconnect();
  }
}

void Pipeline::connect()
{
  vtkAnnotationLayers* annLayers = vtkAnnotationLayers::SafeDownCast(
    this->Reader->GetOutputDataObject(1));
  this->Representation->GetAnnotationLink()->SetAnnotationLayers(
    annLayers);
  this->View->SetAnnotatedGraphRepresentation(this->Representation);
  this->View->SetLayoutStrategyToPassThrough(); // Otherwise it gets reset?
  this->View->Render();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::worldSizeChanged()
{
  if (this->Reader->GetFileName())
  {
  this->Layout->Update();
  double* bb = this->Layout->GetOutput()->GetBounds();
  this->Widget->SetWorldSize(
      sqrt(
          ((bb[1] - bb[0]) * (bb[1] - bb[0]))
          + ((bb[3] - bb[2]) * (bb[3] - bb[2]))));
  }
}

void Pipeline::disconnect()
{
  this->Widget->EnabledOff();
  this->Representation->GetAnnotationLink()->SetAnnotationLayers(0);
  this->View->RemoveAllRepresentations();
  this->View->Render();
}

void Pipeline::saveAnnotationsToFile(const char* fileName)
{
  vtkAnnotationLayers* layers =
      this->Representation->GetAnnotationLink()->GetAnnotationLayers();
  vtkIndent indent;
  std::ofstream os(fileName);
  if (os.is_open())
  {
    vtkAnnotationLayersSerializer::PrintXML(os, indent, 1, layers);
    os.close();
  }
}

void Pipeline::loadAnnotationsFromFile(const char* fileName)
{
  vtkAnnotationLayers* newLayers = vtkAnnotationLayers::New();
  std::ifstream is(fileName);
  if (is.is_open())
  {
    std::stringstream ss;
    char c;
    while (is.good())
    {
      c = is.get();
      if (is.good())
      {
        ss << c;
      }
    }
    is.close();

    vtkAnnotationLayersSerializer::Parse(ss.str().c_str(), newLayers);
    vtkAnnotationLayers* layers =
        this->Representation->GetAnnotationLink()->GetAnnotationLayers();
    for (unsigned int i = 0; i < newLayers->GetNumberOfAnnotations(); ++i)
    {
      layers->AddAnnotation(newLayers->GetAnnotation(i));
    }
    newLayers->Delete();
  }
}

//-----------------------------------------------------------------------------
QImage Pipeline::windowToImage(int zoom)
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<
      vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->renderWindow());
  windowToImageFilter->SetMagnification(zoom);
  windowToImageFilter->SetInputBufferTypeToRGBA();
  windowToImageFilter->Update();
  vtkImageData* imageData = windowToImageFilter->GetOutput();
  int width = imageData->GetDimensions()[0];
  int height = imageData->GetDimensions()[1];
  QImage image(width, height, QImage::Format_ARGB32);
  vtkImageDataToQImage(imageData, image);
  return image;
}

void Pipeline::vtkImageDataToQImage(vtkImageData* imageData, QImage& image)
{
  QRgb* rgbPtr = reinterpret_cast<QRgb*>(image.bits())
          + image.width() * (image.height() - 1);
  unsigned char* colorsPtr =
      reinterpret_cast<unsigned char*>(imageData->GetScalarPointer());
  // mirror vertically
  for (int row = 0; row < image.height(); ++row)
  {
    for (int col = 0; col < image.width(); ++col)
    {
      // Swap rgb
      *(rgbPtr++) = QColor(colorsPtr[0], colorsPtr[1], colorsPtr[2],
          colorsPtr[3]).rgba();
      colorsPtr += 4;
    }
    rgbPtr -= image.width() * 2;
  }
}

//-----------------------------------------------------------------------------
void Pipeline::setLayoutToOGDF(const char* name)
{
#ifdef __USE_OGDF__
  vtkOGDFLayoutStrategy* strategy = vtkOGDFLayoutStrategy::New();
  strategy->LayoutEdgesOn();
  strategy->SetLayoutModuleByName(name);
  this->Layout->SetLayoutStrategy(strategy);
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
#endif
}

void Pipeline::setLayoutToCircular()
{
  vtkCircularLayoutStrategy* strategy = vtkCircularLayoutStrategy::New();
  this->Layout->SetLayoutStrategy(strategy);
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::setLayoutToCone()
{
  bool dfs = true;
  vtkMySpanTreeLayoutStrategy* strategy = vtkMySpanTreeLayoutStrategy::New();
  vtkConeLayoutStrategy* treeLayout = vtkConeLayoutStrategy::New();
  strategy->SetDepthFirstSpanningTree(dfs);
  strategy->SetTreeLayout(treeLayout);

  this->Layout->SetLayoutStrategy(strategy);
  treeLayout->Delete();
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::setLayoutToSimpleTree()
{
  bool dfs = true;
  vtkMySpanTreeLayoutStrategy* strategy = vtkMySpanTreeLayoutStrategy::New();
  vtkTreeLayoutStrategy* treeLayout = vtkTreeLayoutStrategy::New();
  strategy->SetDepthFirstSpanningTree(dfs);
  strategy->SetTreeLayout(treeLayout);

  this->Layout->SetLayoutStrategy(strategy);
  treeLayout->Delete();
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::setLayoutToForceDirected()
{
  vtkForceDirectedLayoutStrategy* strategy =
      vtkForceDirectedLayoutStrategy::New();
  this->Layout->SetLayoutStrategy(strategy);
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::setLayoutToClustering2D()
{
  vtkClustering2DLayoutStrategy* strategy =
      vtkClustering2DLayoutStrategy::New();
  this->Layout->SetLayoutStrategy(strategy);
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

void Pipeline::setLayoutToRandom()
{
  vtkRandomLayoutStrategy* strategy = vtkRandomLayoutStrategy::New();
  strategy->ThreeDimensionalLayoutOff();
  strategy->SetGraphBounds(-100.0, 100.0, -100.0, 100.0, 0.0, 0.0);
  this->Layout->SetLayoutStrategy(strategy);
  strategy->Delete();
  this->View->ResetCamera();
  worldSizeChanged();
}

//-----------------------------------------------------------------------------
void Pipeline::setGraphVertexLabelsOn()
{
  this->Representation->SetVertexLabelVisibility(true);
}

void Pipeline::setGraphVertexLabelsOff()
{
  this->Representation->SetVertexLabelVisibility(false);
}

void Pipeline::setGraphVertexLabelArrayName(const QString& name)
{
  if (name == "- None -")
  {
    this->Representation->SetVertexLabelVisibility(false);
  }
  else
  {
    this->Representation->SetVertexLabelArrayName(
        name.toStdString().c_str());
    this->Representation->SetVertexLabelVisibility(true);
  }
}

void Pipeline::setGraphEdgeLabelsOn()
{
  this->Representation->SetEdgeLabelVisibility(true);
}

void Pipeline::setGraphEdgeLabelsOff()
{
  this->Representation->SetEdgeLabelVisibility(false);
}

void Pipeline::setGraphEdgeLabelArrayName(const QString& name)
{
  if (name == "- None -")
  {
    this->Representation->SetEdgeLabelVisibility(false);
  }
  else
  {
    this->Representation->SetEdgeLabelArrayName(
        name.toStdString().c_str());
    this->Representation->SetEdgeLabelVisibility(true);
  }
}

void Pipeline::setLandmarkLabelsOn()
{
  this->Representation->SetLandmarkLabelArrayName("Hull name");
  this->Representation->SetLandmarkLabelVisibility(true);
}

void Pipeline::setLandmarkLabelsOff()
{
  this->Representation->SetLandmarkLabelVisibility(false);
}

//-----------------------------------------------------------------------------
void Pipeline::setOffScreenProjectionOn()
{
  this->Widget->SetInteractor(this->View->GetInteractor());
  this->Widget->EnabledOn();
}

void Pipeline::setOffScreenProjectionOff()
{
  this->Widget->EnabledOff();
}

void Pipeline::setWidgetAutoDimOn()
{
  this->Widget->AutoDimOn();
}

void Pipeline::setWidgetAutoDimOff()
{
  this->Widget->AutoDimOff();
}

void Pipeline::setOverlapReductionOn()
{
  this->Widget->SetReduceOverlaps(true);
}

void Pipeline::setOverlapReductionOff()
{
  this->Widget->SetReduceOverlaps(false);
}

void Pipeline::setShowErrorOn()
{
  this->Widget->SetShowError(true);
}

void Pipeline::setShowErrorOff()
{
  this->Widget->SetShowError(false);
}

//-----------------------------------------------------------------------------
void Pipeline::setLandmarkScaling(int scale)
{
  this->Representation->SetScaleFactor(scale / 100.0);
}

void Pipeline::setMinimumSize(unsigned size)
{
  this->Representation->SetMinimumSize(size);
}

void Pipeline::setLandmarkOutlineOn(bool b)
{
  this->Representation->SetOutlineVisibility(b);
}

void Pipeline::setLandmarkToConvexHull()
{
  this->Representation->SetLandmarkToConvexHull();
}

void Pipeline::setLandmarkToRectangle()
{
  this->Representation->SetLandmarkToRectangle();
}
//----------------------------------------------------------------------------
void Pipeline::setFlightSpeedBias(int bias)
{
  this->Widget->SetFlightSpeedBias(bias);
}

void Pipeline::setFlightTypeToExpress()
{
  this->Widget->SetFlightTypeToExpress();
}

void Pipeline::setFlightTypeToTourist()
{
  this->Widget->SetFlightTypeToTourist();
}

//----------------------------------------------------------------------------
void Pipeline::setBackgroundColour(const QColor& color)
{
  this->View->GetRenderer()->SetBackground(color.redF(), color.greenF(),
      color.blueF());
  this->View->GetRenderer()->SetBackground2(color.redF(), color.greenF(),
      color.blueF());
}

QColor Pipeline::backgroundColour()
{
  double rgb[3];
  this->View->GetRenderer()->GetBackground(rgb);
  QColor color;
  color.setRedF(rgb[0]);
  color.setGreenF(rgb[1]);
  color.setBlueF(rgb[2]);
  return color;
}

void Pipeline::setGraphVertexColour(const QColor& color)
{
  this->Representation->SetDefaultVertexColor(color.redF(), color.greenF(),
      color.blueF(), color.alphaF());
}

QColor Pipeline::graphVertexColour()
{
  double rgba[4];
  this->Representation->GetDefaultVertexColor(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setGraphEdgeColour(const QColor& color)
{
  this->Representation->SetDefaultEdgeColor(color.redF(), color.greenF(),
      color.blueF(), color.alphaF());
}

QColor Pipeline::graphEdgeColour()
{
  double rgba[4];
  this->Representation->GetDefaultEdgeColor(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setGraphVertexLabelColour(const QColor& color)
{
  this->Representation->GetVertexLabelTextProperty()->SetColor(
      color.redF(), color.greenF(), color.blueF());
  this->Representation->GetVertexLabelTextProperty()->SetOpacity(
      color.alphaF());
}

QColor Pipeline::graphVertexLabelColour()
{
  double rgba[4];
  this->Representation->GetVertexLabelTextProperty()->GetColor(rgba[0],
      rgba[1], rgba[2]);
  rgba[3] =
      this->Representation->GetVertexLabelTextProperty()->GetOpacity();
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setGraphEdgeLabelColour(const QColor& color)
{
  this->Representation->GetEdgeLabelTextProperty()->SetColor(color.redF(),
      color.greenF(), color.blueF());
  this->Representation->GetEdgeLabelTextProperty()->SetOpacity(
      color.alphaF());
}

QColor Pipeline::graphEdgeLabelColour()
{
  double rgba[4];
  this->Representation->GetEdgeLabelTextProperty()->GetColor(rgba[0],
      rgba[1], rgba[2]);
  rgba[3] = this->Representation->GetEdgeLabelTextProperty()->GetOpacity();
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setLandmarkOutlineColour(const QColor& color)
{

  this->Representation->SetLandmarkOutlineColour(color.redF(),
      color.greenF(), color.blueF(), color.alphaF());
}

QColor Pipeline::landmarkOutlineColour()
{
  double rgba[4];
  this->Representation->GetLandmarkOutlineColour(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setLandmarkLabelColour(const QColor& color)
{

  this->Representation->GetLandmarkLabelTextProperty()->SetColor(
      color.redF(), color.greenF(), color.blueF());
  this->Representation->GetLandmarkLabelTextProperty()->SetOpacity(
      color.alphaF());
}

QColor Pipeline::landmarkLabelColour()
{
  double rgba[4];
  this->Representation->GetLandmarkLabelTextProperty()->GetColor(rgba[0],
      rgba[1], rgba[2]);
  rgba[3] =
      this->Representation->GetLandmarkLabelTextProperty()->GetOpacity();
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setBezelColour(const QColor& color)
{
  //  this->GraphRepresentation->SetBezelColour(color.redF(), color.greenF(),
  //      color.blueF(), color.alphaF());
}

QColor Pipeline::bezelColour()
{
  double rgba[4];
  //  this->GraphRepresentation->GetBezelColour(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setSelectedVertexColour(const QColor& color)
{
  this->Representation->SetSelectedVertexColour(color.redF(),
      color.greenF(), color.blueF(), color.alphaF());
}

QColor Pipeline::selectedVertexColour()
{
  double rgba[4];
  this->Representation->GetSelectedVertexColour(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setSelectedEdgeColour(const QColor& color)
{
  this->Representation->SetSelectedEdgeColour(color.redF(), color.greenF(),
      color.blueF(), color.alphaF());
}

QColor Pipeline::selectedEdgeColour()
{
  double rgba[4];
  this->Representation->GetSelectedEdgeColour(rgba);
  QColor color;
  color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
  return color;
}

void Pipeline::setUseCurrentAnnotationColourOn()
{
  this->Representation->SetUseCurrentAnnotationColour(true);
}

void Pipeline::setUseCurrentAnnotationColourOff()
{
  this->Representation->SetUseCurrentAnnotationColour(false);
}

bool Pipeline::useCurrentAnnotationColour()
{
  return this->Representation->GetUseCurrentAnnotationColour();
}

//----------------------------------------------------------------------------
QStringList Pipeline::graphVertexLabelArrayNames()
{
  QStringList qNames;
  qNames.append("- None -");
  vtkStringArray* names = this->Representation->GetVertexArrayNames();
  for (vtkIdType i = 0; i < names->GetNumberOfValues(); ++i)
  {
    qNames.append(QString(names->GetValue(i)));
  }
  names->Delete();
  return qNames;
}

QStringList Pipeline::graphEdgeLabelArrayNames()
{
  QStringList qNames;
  qNames.append("- None -");
  vtkStringArray* names = this->Representation->GetEdgeArrayNames();
  for (vtkIdType i = 0; i < names->GetNumberOfValues(); ++i)
  {
    qNames.append(QString(names->GetValue(i)));
  }
  names->Delete();
  return qNames;
}
