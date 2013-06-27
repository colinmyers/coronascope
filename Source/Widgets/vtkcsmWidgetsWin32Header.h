/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkcsmWidgetsWin32Header.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkcsmWidgetsWin32Header - manage Windows system differences
// .SECTION Description
// The vtkcsmWidgetsWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkcsmWidgetsWin32Header_h
#define __vtkcsmWidgetsWin32Header_h

#include <vtkcsmConfigure.h>

#if defined(WIN32) && !defined(VTK_MY_STATIC)
#if defined(vtkcsmWidgets_EXPORTS)
#define VTK_CSM_WIDGETS_EXPORT __declspec( dllexport )
#else
#define VTK_CSM_WIDGETS_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_CSM_WIDGETS_EXPORT
#endif

#endif
