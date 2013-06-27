/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkcsmViewsWin32Header.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkcsmViewsWin32Header - manage Windows system differences
// .SECTION Description
// The vtkcsmViewsWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkcsmViewsWin32Header_h
#define __vtkcsmViewsWin32Header_h

#include <vtkcsmConfigure.h>

#if defined(WIN32) && !defined(VTK_MY_STATIC)
#if defined(vtkcsmViews_EXPORTS)
#define VTK_CSM_VIEWS_EXPORT __declspec( dllexport )
#else
#define VTK_CSM_VIEWS_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_CSM_VIEWS_EXPORT
#endif

#endif
