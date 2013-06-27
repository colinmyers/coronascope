/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkMyQtAnnotationLayersModelAdapter.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
/*-------------------------------------------------------------------------
 Copyright 2008 Sandia Corporation.
 Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 the U.S. Government retains certain rights in this software.
 -------------------------------------------------------------------------*/
#include "vtkMyQtAnnotationLayersModelAdapter.h"

#include "vtkAnnotation.h"
#include "vtkAnnotationLayers.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkStdString.h"
#include "vtkVariant.h"

#include <QtGui/QColor>

//----------------------------------------------------------------------------
vtkMyQtAnnotationLayersModelAdapter::vtkMyQtAnnotationLayersModelAdapter(
    QObject* p) :
    vtkQtAbstractModelAdapter(p)
{
  this->Annotations = NULL;
}

//----------------------------------------------------------------------------
vtkMyQtAnnotationLayersModelAdapter::vtkMyQtAnnotationLayersModelAdapter(
    vtkAnnotationLayers* t, QObject* p) :
        vtkQtAbstractModelAdapter(p), Annotations(t)
{
  if (this->Annotations != NULL)
  {
    this->Annotations->Register(0);
  }
}

//----------------------------------------------------------------------------
vtkMyQtAnnotationLayersModelAdapter::~vtkMyQtAnnotationLayersModelAdapter()
{
  if (this->Annotations != NULL)
  {
    this->Annotations->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkMyQtAnnotationLayersModelAdapter::SetVTKDataObject(vtkDataObject *obj)
{
  vtkAnnotationLayers *t = vtkAnnotationLayers::SafeDownCast(obj);
  if (obj && !t)
  {
    qWarning(
        "vtkMyQtAnnotationLayersModelAdapter needs a vtkAnnotationLayers for SetVTKDataObject");
    return;
  }

  // Okay it's a table so set it :)
  this->setAnnotationLayers(t);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkMyQtAnnotationLayersModelAdapter::GetVTKDataObject() const
{
  return this->Annotations;
}

//----------------------------------------------------------------------------
void vtkMyQtAnnotationLayersModelAdapter::setAnnotationLayers(
    vtkAnnotationLayers* t)
{
  if (this->Annotations != NULL)
  {
    this->Annotations->Delete();
  }
  this->Annotations = t;
  if (this->Annotations != NULL)
  {
    this->Annotations->Register(0);
    emit this->reset();
  }
}

//----------------------------------------------------------------------------
vtkAnnotationLayers* vtkMyQtAnnotationLayersModelAdapter::annotationLayers() const
{
  return this->Annotations;
}

//----------------------------------------------------------------------------
bool vtkMyQtAnnotationLayersModelAdapter::annotationsIsEmpty() const
{
  if (this->Annotations == NULL)
  {
    // It's not necessarily an error to have a null pointer for the
    // table.  It just means that the model is empty.
    return true;
  }
  if (this->Annotations->GetNumberOfAnnotations() == 0)
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
QVariant vtkMyQtAnnotationLayersModelAdapter::data(const QModelIndex &idx,
    int role) const
{
  if (this->annotationsIsEmpty())
  {
    return QVariant();
  }
  if (!idx.isValid())
  {
    return QVariant();
  }
  if (idx.row()
      >= static_cast<int>(this->Annotations->GetNumberOfAnnotations()))
  {
    return QVariant();
  }

  vtkAnnotation *a = this->Annotations->GetAnnotation(idx.row());
  int numItems = 0;
  vtkSelection *s = a->GetSelection();
  if (s)
  {
    // Only count selection types we are interested in (vertex/indices)
    for (unsigned int i = 0; i < s->GetNumberOfNodes(); ++i)
    {
      if (s->GetNode(i)->GetContentType() == vtkSelectionNode::INDICES
          && s->GetNode(i)->GetFieldType() == vtkSelectionNode::VERTEX)
      {
        numItems += 1;
      }
    }
  }

  int enabled = a->GetInformation()->Get(vtkAnnotation::ENABLE());
  double *color = a->GetInformation()->Get(vtkAnnotation::COLOR());
  double opacity = a->GetInformation()->Get(vtkAnnotation::OPACITY());
  int annColor[4];
  annColor[0] = static_cast<int>(255 * color[0]);
  annColor[1] = static_cast<int>(255 * color[1]);
  annColor[2] = static_cast<int>(255 * color[2]);
  annColor[3] = static_cast<int>(255 * opacity);

  if (role == Qt::CheckStateRole && idx.column() == 3)
  {
    if (enabled == 1)
    {
      return Qt::Checked;
    }
    else
    {
      return Qt::Unchecked;
    }
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    switch (idx.column())
    {
    case 1:
      return QVariant(a->GetInformation()->Get(vtkAnnotation::LABEL()));
      break;
    case 2:
      return QVariant(numItems);
      break;
    default:
      return QVariant();
      break;
    }
  }
  else if (role == Qt::DecorationRole)
  {
    switch (idx.column())
    {
    case 0:
      return QColor(annColor[0], annColor[1], annColor[2], annColor[3]);
      break;
    default:
      return QVariant();
      break;
    }
  }

  return QVariant();
}

//----------------------------------------------------------------------------
bool vtkMyQtAnnotationLayersModelAdapter::setData(const QModelIndex &idx,
    const QVariant &value, int role)
{

  if (role == Qt::CheckStateRole)
  {
    if (value == Qt::Checked)
    {
      // Set enabled
      vtkAnnotation* annotation = this->Annotations->GetAnnotation(idx.row());
      annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), true);
      emit this->dataChanged(idx, idx);
      return true;
    }
    else
    {
      // Set enabled
      vtkAnnotation* annotation = this->Annotations->GetAnnotation(idx.row());
      annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), false);
      emit this->dataChanged(idx, idx);
      return true;
    }
  }

  if (role == Qt::EditRole)
  {
    if (idx.column() == 0)
    {
      // Set color and opacity
      QColor color = value.value<QColor>();
      vtkAnnotation* annotation = this->Annotations->GetAnnotation(idx.row());
      annotation->GetInformation()->Set(vtkAnnotation::COLOR(), color.redF(),
          color.greenF(), color.blueF());
      annotation->GetInformation()->Set(vtkAnnotation::OPACITY(),
          color.alphaF());
      emit this->dataChanged(idx, idx);
      return true;
    }
    else if (idx.column() == 1)
    {
      // Set label
      vtkAnnotation* annotation = this->Annotations->GetAnnotation(idx.row());
      annotation->GetInformation()->Set(vtkAnnotation::LABEL(),
          value.toString().toStdString().c_str());
      emit this->dataChanged(idx, idx);
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
Qt::ItemFlags vtkMyQtAnnotationLayersModelAdapter::flags(
    const QModelIndex &idx) const
{
  if (!idx.isValid())
  {
    return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
  }

  if (idx.column() == 1)
  {
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
  }
  else if (idx.column() == 3)
  {
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable
        | Qt::ItemIsUserCheckable;
  }
  else
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

}

//----------------------------------------------------------------------------
QVariant vtkMyQtAnnotationLayersModelAdapter::headerData(int section,
    Qt::Orientation orientation, int role) const
{
  if (this->annotationsIsEmpty())
  {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
    case 0:
      return QVariant("Colour");
      break;
    case 1:
      return QVariant("Label");
      break;
    case 2:
      return QVariant("Number of landmarks");
      break;
    case 3:
      return QVariant("Enabled");
      break;
    default:
      return QVariant();
      break;
    }
  }
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return QVariant(section + 1);
  }

  return QVariant();
}

//----------------------------------------------------------------------------
QModelIndex vtkMyQtAnnotationLayersModelAdapter::index(int row, int column,
    const QModelIndex &parentIdx) const
{
  if (this->annotationsIsEmpty())
  {
    return QModelIndex();
  }

  return createIndex(row, column, row);
}

//----------------------------------------------------------------------------
QModelIndex vtkMyQtAnnotationLayersModelAdapter::parent(
    const QModelIndex & idx) const
{
  return QModelIndex();
}

//----------------------------------------------------------------------------
int vtkMyQtAnnotationLayersModelAdapter::rowCount(const QModelIndex & idx) const
{
  if (this->annotationsIsEmpty())
  {
    return 0;
  }
  return this->Annotations->GetNumberOfAnnotations();
}

//----------------------------------------------------------------------------
int vtkMyQtAnnotationLayersModelAdapter::columnCount(const QModelIndex &) const
{
  return 4;
}

//----------------------------------------------------------------------------
bool vtkMyQtAnnotationLayersModelAdapter::insertRows(int row, int count,
    const QModelIndex &p)
{
  emit this->beginInsertRows(p, row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    vtkSmartPointer<vtkAnnotation> annotation =
        vtkSmartPointer<vtkAnnotation>::New();
    vtkSelection* selection = vtkSelection::New();
    vtkSelectionNode* selNode = vtkSelectionNode::New();
    selNode->SetContentType(vtkSelectionNode::INDICES);
    vtkIdTypeArray* selList = vtkIdTypeArray::New();
    selNode->SetSelectionList(selList);
    selection->AddNode(selNode);
    annotation->SetSelection(selection);
    selList->Delete();
    selNode->Delete();
    selection->Delete();

    double defaultColor[3] =
    { 0.0, 0.0, 1.0 };
    annotation->GetInformation()->Set(vtkAnnotation::COLOR(), defaultColor, 3);
    annotation->GetInformation()->Set(vtkAnnotation::OPACITY(), 0.4);
    annotation->GetInformation()->Set(vtkAnnotation::LABEL(), "New");
    annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), 1);

    this->Annotations->AddAnnotation(annotation);
  }

  emit this->endInsertRows();

  return true;
}

//----------------------------------------------------------------------------
bool vtkMyQtAnnotationLayersModelAdapter::removeRows(int row, int count,
    const QModelIndex &p)
{
  emit this->beginRemoveRows(p, row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    this->Annotations->RemoveAnnotation(this->Annotations->GetAnnotation(row));
  }
  emit this->endRemoveRows();

  return true;
}
