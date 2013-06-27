#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "vtkSmartPointer.h"

class QColor;
class QImage;
class QString;
class QStringList;

class vtkAnnotatedGraphView;
class vtkAnnotatedGraphRepresentation;
class vtkAnnotation;
class vtkFlightMapFilter;
class vtkGraphLayout;
class vtkImageData;
class vtkOffScreenWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkTulipReader;

//! A VTK pipeline to create an annotated graph view and off-screen widget.
/*!
  This class instantiates and modifies a VTK pipeline and provides an interface
  to Qt. Most methods simply get/set parameters on the pipeline filters and
  provide conversion between respective data object types. The pipeline consists
  of:
  <pre>
  TulipReader -- GraphLayout -- AnnotatedGraphRepresentation -- AnnotatedGraphView
  </pre>
  Additionally, a vtkOffScreenWidget is set on the view. Most of the pipeline
  connection is performed in the constructor, with final preparations made in
  the connect() method.
 */
class Pipeline
{
public:
  Pipeline();
  ~Pipeline();

  vtkAnnotatedGraphView* view() const;
  vtkRenderWindow* renderWindow() const;
  void setInteractor(vtkRenderWindowInteractor* iren);
  QImage windowToImage(int zoom);

  //!@{
  //! File handling
  void setFileName(const char* fileName);
  void saveAnnotationsToFile(const char* fileName);
  void loadAnnotationsFromFile(const char* fileName);
  //!@}

  //!@{
  //! Set layout strategy
  void setLayoutToCircular();
  void setLayoutToOGDF(const char* name);
  void setLayoutToCone();
  void setLayoutToSimpleTree();
  void setLayoutToForceDirected();
  void setLayoutToClustering2D();
  void setLayoutToRandom();
  //!@}

  //!@{
  //! Text labels
  void setGraphVertexLabelsOn();
  void setGraphVertexLabelsOff();
  void setGraphVertexLabelArrayName(const QString& name);
  QStringList graphVertexLabelArrayNames();

  void setGraphEdgeLabelsOn();
  void setGraphEdgeLabelsOff();
  void setGraphEdgeLabelArrayName(const QString& name);
  QStringList graphEdgeLabelArrayNames();

  void setLandmarkLabelsOn();
  void setLandmarkLabelsOff();
  //!@}

  //!@{
  //! Off-Screen Widget
  void setOffScreenProjectionOn();
  void setOffScreenProjectionOff();
  void setWidgetAutoDimOn();
  void setWidgetAutoDimOff();
  void setOverlapReductionOn();
  void setOverlapReductionOff();
  void setShowErrorOn();
  void setShowErrorOff();

  void setFlightSpeedBias(int bias);
  void setFlightTypeToExpress();
  void setFlightTypeToTourist();
  //!@}

  //!@{
  //! Graph Annotations
  void setLandmarkScaling(int scale);
  void setMinimumSize(unsigned size);
  void setLandmarkOutlineOn(bool b);
  void setLandmarkToConvexHull();
  void setLandmarkToRectangle();
  //!@}

  //!@{
  //! Colours
  void setBackgroundColour(const QColor& color);
  QColor backgroundColour();

  void setGraphVertexColour(const QColor& color);
  QColor graphVertexColour();

  void setGraphEdgeColour(const QColor& color);
  QColor graphEdgeColour();

  void setGraphVertexLabelColour(const QColor& color);
  QColor graphVertexLabelColour();

  void setGraphEdgeLabelColour(const QColor& color);
  QColor graphEdgeLabelColour();

  void setLandmarkOutlineColour(const QColor& color);
  QColor landmarkOutlineColour();

  void setLandmarkLabelColour(const QColor& color);
  QColor landmarkLabelColour();

  void setBezelColour(const QColor& color);
  QColor bezelColour();

  void setSelectedVertexColour(const QColor& color);
  QColor selectedVertexColour();

  void setSelectedEdgeColour(const QColor& color);
  QColor selectedEdgeColour();

  void setUseCurrentAnnotationColourOn();
  void setUseCurrentAnnotationColourOff();
  bool useCurrentAnnotationColour();
  //!@}

protected:
  //! Connect the pipeline and generate the view.
  void connect();

  //! Disconnect the pipeline and blank the view.
  void disconnect();

  //! Update the world size needed by the Widget to calculate pointer length.
  void worldSizeChanged();

  //! Conveniently convert a vtkImageData to a QImage.
  void vtkImageDataToQImage(vtkImageData* imageData, QImage& image);

private:
  Pipeline(const Pipeline&); // Not implemented
  Pipeline operator=(const Pipeline&); // Not implemented

  vtkSmartPointer<vtkTulipReader> Reader;
  vtkSmartPointer<vtkGraphLayout> Layout;
  vtkSmartPointer<vtkAnnotatedGraphRepresentation> Representation;
  vtkSmartPointer<vtkAnnotatedGraphView> View;
  vtkSmartPointer<vtkOffScreenWidget> Widget;
  vtkSmartPointer<vtkFlightMapFilter> FlightMap;
};

#endif /* PIPELINE_H_ */
