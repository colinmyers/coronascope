#include "vtkAnnotatedGraphView.h"
#include "vtkAnnotationLink.h"
#include "vtkDataRepresentation.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkTulipReader.h"

/*
 * This example shows how to read an annotated graph from a file and
 * display it using a vtkAnnotatedGraphView.
 */

int main(int argc, char* argv[])
{
  // Create a Tulip file reader and read in an annotated graph
  // (In Tulip the annotations are referred to as clusters).
  vtkSmartPointer<vtkTulipReader> reader =
      vtkSmartPointer<vtkTulipReader>::New();
  reader->SetFileName("../../Data/ten_nodes.tlp");

  // Create the view and set a nice layout
  vtkSmartPointer<vtkAnnotatedGraphView> view =
      vtkSmartPointer<vtkAnnotatedGraphView>::New();
  view->SetLayoutStrategyToForceDirected();
  view->SetRepresentationFromInputConnection(reader->GetOutputPort());

  // Plug the annotation layers data from the reader in to the view.
  view->GetRepresentation()->GetAnnotationLink()
      ->AddInputConnection(reader->GetOutputPort(1));

  // Reset the camera position and show.
  view->Render();
  view->ResetCamera();
  view->GetInteractor()->Start();

  return EXIT_SUCCESS;
}
