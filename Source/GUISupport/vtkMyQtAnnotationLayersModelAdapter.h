/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkMyQtAnnotationLayersModelAdapter.h

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
// .NAME vtkMyQtAnnotationLayersModelAdapter - Adapts annotations to a Qt item model.
//
// .SECTION Description
// vtkMyQtAnnotationLayersModelAdapter is a QAbstractItemModel with a
// vtkAnnotationLayers as its underlying data model. This version is customised
// for use with the annotation data layout produced by a
// vtkGraphAnnotationLayersFilter.
//
// .SECTION See also
// vtkQtAbstractModelAdapter vtkQtTableModelAdapter

#ifndef __vtkMyQtAnnotationLayersModelAdapter_h
#define __vtkMyQtAnnotationLayersModelAdapter_h

#include "QVTKWin32Header.h"
#include "vtkQtAbstractModelAdapter.h"

class vtkAnnotationLayers;

class QVTK_EXPORT vtkMyQtAnnotationLayersModelAdapter:
  public vtkQtAbstractModelAdapter
{
  Q_OBJECT

public:
  vtkMyQtAnnotationLayersModelAdapter(QObject *parent = 0);
  vtkMyQtAnnotationLayersModelAdapter(vtkAnnotationLayers* ann,
      QObject *parent = 0);
  virtual ~vtkMyQtAnnotationLayersModelAdapter();

  // Description:
  // Set/Get the VTK data object as input to this adapter
  virtual void SetVTKDataObject(vtkDataObject *data);
  virtual vtkDataObject* GetVTKDataObject() const;

  // Description:
  // Set/Get the annotation layers data object
  void setAnnotationLayers(vtkAnnotationLayers* annotations);
  vtkAnnotationLayers* annotationLayers() const;

  // Description:
  // Qt model override methods.
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role =
      Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role =
      Qt::DisplayRole) const;
  QModelIndex index(int row, int column, const QModelIndex &parent =
      QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  bool removeRows(int row, int count, const QModelIndex & parent =
      QModelIndex());
  bool insertRows(int row, int count, const QModelIndex & parent =
      QModelIndex());

  // Description:
  // Virtual abstract methods that are not used in this class.
  virtual vtkSelection* QModelIndexListToVTKIndexSelection(
    const QModelIndexList qmil) const { return 0; };
  virtual QItemSelection VTKIndexSelectionToQItemSelection(
    vtkSelection *vtksel) const { return QItemSelection(); }
  virtual void SetKeyColumnName(const char* name) { };
  virtual void SetColorColumnName(const char* name) { };


private:
  // Description:
  // Test to see if there are any annotations
  // (i.e. null or empty vtkAnnotationLayers).
  bool annotationsIsEmpty() const;
  vtkAnnotationLayers* Annotations;

  vtkMyQtAnnotationLayersModelAdapter(
      const vtkMyQtAnnotationLayersModelAdapter &);  // Not implemented
  void operator=(const vtkMyQtAnnotationLayersModelAdapter&); // Not implemented.
};

#endif
