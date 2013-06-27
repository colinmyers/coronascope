/*=========================================================================

  Program:   ParaView
  Module:    vtkAnnotationLayersSerializer.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAnnotationLayersSerializer - Serialize/deserialize vtkAnnotationLayers to/from xml
// .SECTION Description
// vtkAnnotationLayersSerializer is a helper class that can
// serialize/deserialize vtkAnnotationLayers to/from xml.
// .SECTION See Also
// vtkAnnotationLayers

#ifndef __vtkAnnotationLayersSerializer_h
#define __vtkAnnotationLayersSerializer_h

#include "vtkcsmIOWin32Header.h"
#include "vtkObject.h"

class vtkInformationIntegerKey;
class vtkPVXMLElement;
class vtkAnnotationLayers;
class vtkAnnotation;

class VTK_IO_EXPORT vtkAnnotationLayersSerializer : public vtkObject
{
public:
  static vtkAnnotationLayersSerializer* New();
  vtkTypeMacro(vtkAnnotationLayersSerializer,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Serialize the selection tree to a stream as xml.
  // For now, only keys of type vtkInformationIntegerKey are supported.
  static void PrintXML(int printData, vtkAnnotationLayers* annotationLayers);
  static void PrintXML(ostream& os, 
                       vtkIndent indent, 
                       int printData, 
                       vtkAnnotationLayers* annotationLayers);

  // Description:
  // Parse an xml string to create a new vtkAnnotationLayers.
  static void Parse(const char* xml, vtkAnnotationLayers* annotationLayers);
protected:
  vtkAnnotationLayersSerializer();
  ~vtkAnnotationLayersSerializer();

private:
  vtkAnnotationLayersSerializer(const vtkAnnotationLayersSerializer&);  // Not implemented.
  void operator=(const vtkAnnotationLayersSerializer&);  // Not implemented.

  static void WriteSelection(ostream& os,
                             vtkIndent indent,
                             vtkAnnotation* annotation);
  static void ParseAnnotation(
    vtkPVXMLElement* nodeXML, vtkAnnotation* annotation);
};

#endif
