/*=========================================================================

  Program:   ParaView
  Module:    vtkAnnotationLayersSerializer.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAnnotationLayersSerializer.h"

#include "vtkAnnotation.h"
#include "vtkAnnotationLayers.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkInformationIterator.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationKey.h"
#include "vtkInstantiator.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLParser.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSerializer.h"
#include "vtkStringArray.h"

#include <string>
#include <sstream>

vtkStandardNewMacro(vtkAnnotationLayersSerializer);

//vtkInformationKeyMacro(vtkSelectionSerializer,ORIGINAL_SOURCE_ID,Integer);

//----------------------------------------------------------------------------
vtkAnnotationLayersSerializer::vtkAnnotationLayersSerializer()
{
}

//----------------------------------------------------------------------------
vtkAnnotationLayersSerializer::~vtkAnnotationLayersSerializer()
{
}

//----------------------------------------------------------------------------
void vtkAnnotationLayersSerializer::PrintXML(int printData,
  vtkAnnotationLayers* annotationLayers)
{
  vtkAnnotationLayersSerializer::PrintXML(cout, vtkIndent(), printData, annotationLayers);
}

//----------------------------------------------------------------------------
void vtkAnnotationLayersSerializer::PrintXML(
  ostream& os, vtkIndent indent, int printData, vtkAnnotationLayers* annotationLayers)
{
  os << indent << "<AnnotationLayers>" << endl;
  vtkIndent annotationIndent = indent.GetNextIndent();
  unsigned int numAnnotations = annotationLayers->GetNumberOfAnnotations();
  for (unsigned int i = 0; i < numAnnotations; i++)
    {
    os << annotationIndent << "<Annotation>" << endl;
    vtkAnnotation* annotation = annotationLayers->GetAnnotation(i);

    vtkIndent ni = annotationIndent.GetNextIndent();

    // Write out all properties.
    // For now, only keys of type vtkInformationIntegerKey are supported.
    vtkInformationIterator* iter = vtkInformationIterator::New();
    vtkInformation* properties = annotation->GetInformation();
    iter->SetInformation(properties);
    for(iter->GoToFirstItem();
      !iter->IsDoneWithTraversal();
      iter->GoToNextItem())
      {
      vtkInformationKey* key = iter->GetCurrentKey();
      os << ni
        << "<Property key=\"" << key->GetName();
      if (key->IsA("vtkInformationIntegerKey"))
        {
        vtkInformationIntegerKey* iKey =
          static_cast<vtkInformationIntegerKey*>(key);
        os << "\" value=\"" << properties->Get(iKey);
        }
      else if (key->IsA("vtkInformationDoubleKey"))
        {
        vtkInformationDoubleKey* dKey =
          static_cast<vtkInformationDoubleKey*>(key);
        os << "\" value=\"" << properties->Get(dKey);
        }
      else if (key->IsA("vtkInformationDoubleVectorKey"))
        {
        vtkInformationDoubleVectorKey* dvKey =
          static_cast<vtkInformationDoubleVectorKey*>(key);
        double* rgb = properties->Get(dvKey);
        os << "\" value=\"#";
        os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(rgb[0] * 255);
        os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(rgb[1] * 255);
        os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(rgb[2] * 255);
        os << std::setw(1) << std::dec;
        }
      else if (key->IsA("vtkInformationStringKey"))
        {
        vtkInformationStringKey* sKey =
          static_cast<vtkInformationStringKey*>(key);
        os << "\" value=\"" << properties->Get(sKey);
        }

      os << "\"/>" << endl;
      }
    iter->Delete();

    // Write the selection
    if (printData)
      {
      vtkAnnotationLayersSerializer::WriteSelection(os, ni, annotation);
      }

    os << annotationIndent << "</Annotation>" << endl;
    }

  os << indent << "</AnnotationLayers>" << endl;
}

//----------------------------------------------------------------------------
template <class T>
void vtkSelectionSerializerWriteSelectionList(ostream& os, vtkIndent indent,
  vtkIdType numElems, T* dataPtr)
{
  os << indent;
  for (vtkIdType idx=0; idx<numElems; idx++)
    {
    os << dataPtr[idx] << " ";
    }
  os << endl;
}

//----------------------------------------------------------------------------
// Serializes the selection list data array
void vtkAnnotationLayersSerializer::WriteSelection(
  ostream& os, vtkIndent indent, vtkAnnotation* annotation)
{
  os << indent;
  vtkSelection* selection = annotation->GetSelection();
  vtkSelectionSerializer::PrintXML(os, indent, 1, selection);
}

//----------------------------------------------------------------------------
void vtkAnnotationLayersSerializer::Parse(
  const char* xml, vtkAnnotationLayers* annotationLayers)
{
  annotationLayers->Initialize();

  vtkPVXMLParser* parser = vtkPVXMLParser::New();
  parser->Parse(xml);
  vtkPVXMLElement* rootElem = parser->GetRootElement();
  if (rootElem)
    {
    unsigned int numNested = rootElem->GetNumberOfNestedElements();
    for (unsigned int i=0; i<numNested; i++)
      {
      vtkPVXMLElement* elem = rootElem->GetNestedElement(i);
      const char* name = elem->GetName();
      if (!name)
        {
        continue;
        }
      if (strcmp("AnnotationLayers", name) == 0 )
        {
        annotationLayers->Initialize();
        }
      if (strcmp("Annotation", name) == 0 )
        {
        vtkAnnotation* newAnnotation = vtkAnnotation::New();
        annotationLayers->AddAnnotation(newAnnotation);
        vtkAnnotationLayersSerializer::ParseAnnotation(elem, newAnnotation);
        newAnnotation->Delete();
        }
      }
    }
  parser->Delete();
}

//----------------------------------------------------------------------------
void vtkAnnotationLayersSerializer::ParseAnnotation(
  vtkPVXMLElement* annotationXML, vtkAnnotation* annotation)
{
  if (!annotationXML || !annotation)
    {
    return;
    }

  unsigned int numNested = annotationXML->GetNumberOfNestedElements();
  for (unsigned int i=0; i<numNested; i++)
    {
    vtkPVXMLElement* elem = annotationXML->GetNestedElement(i);
    const char* name = elem->GetName();
    if (!name)
      {
      continue;
      }

    // Only a selected list of keys are supported
    else if (strcmp("Property", name) == 0)
      {
      const char* key = elem->GetAttribute("key");
      if (key)
        {
        if (strcmp("LABEL", key) == 0)
          {
          const char* val = elem->GetAttributeOrEmpty("value");
          if (val)
            {
            annotation->GetInformation()->Set(vtkAnnotation::LABEL(), val);
            }
          }
        else if (strcmp("COLOR", key) == 0)
          {
          std::string colorString(elem->GetAttributeOrEmpty("value"));
          std::stringstream ssRed, ssGreen, ssBlue;
          int red;
          int green;
          int blue;
          ssRed << colorString.substr(0 * 2 + 1, 2);
          ssGreen << colorString.substr(1 * 2 + 1, 2);
          ssBlue << colorString.substr(2 * 2 + 1, 2);
          ssRed >> std::hex >> red;
          ssGreen >> std::hex >> green;
          ssBlue >> std::hex >> blue;
          annotation->GetInformation()->Set(
            vtkAnnotation::COLOR(), red / 255.0, green / 255.0, blue / 255.0);
          }
        else if (strcmp("OPACITY", key) == 0)
          {
          double val;
          if (elem->GetScalarAttribute("value", &val))
            {
            annotation->GetInformation()->Set(vtkAnnotation::OPACITY(), val);
            }
          }
        else if (strcmp("ICON_INDEX", key) == 0)
          {
          int val;
          if (elem->GetScalarAttribute("value", &val))
            {
            annotation->GetInformation()->Set(vtkAnnotation::ICON_INDEX(), val);
            }
          }
        else if (strcmp("ENABLE", key) == 0)
          {
          int val;
          if (elem->GetScalarAttribute("value", &val))
            {
            annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), val);
            }
          }
        else if (strcmp("HIDE", key) == 0)
          {
          int val;
          if (elem->GetScalarAttribute("value", &val))
            {
            annotation->GetInformation()->Set(vtkAnnotation::HIDE(), val);
            }
          }
        }
      }
    else if (strcmp("Selection", name) == 0)
      {
      vtkSelection* selection = vtkSelection::New();

      unsigned int numNested = elem->GetNumberOfNestedElements();
      for (unsigned int i=0; i<numNested; i++)
        {
        vtkPVXMLElement* nodeElem = elem->GetNestedElement(i);
        const char* name = nodeElem->GetName();
        if (!name)
          {
          continue;
          }
        if (strcmp("Selection", name) == 0 )
          {
          vtkSelectionNode* newNode = vtkSelectionNode::New();
          selection->AddNode(newNode);
          vtkSelectionSerializer::ParseNode(nodeElem, newNode);
          newNode->Delete();
          }
        }
      annotation->SetSelection(selection);
      selection->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationLayersSerializer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

