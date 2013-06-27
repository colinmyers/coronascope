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
// .NAME vtkcsmIOWin32Header - manage Windows system differences
// .SECTION Description
// The vtkcsmIOWin32Header captures some system differences between Unix
// and Windows operating systems.

#ifndef __vtkcsmIOWin32Header_h
#define __vtkcsmIOWin32Header_h

#include <vtkcsmConfigure.h>

#if defined(WIN32) && !defined(VTK_MY_STATIC)
#if defined(vtkcsmIO_EXPORTS)
#define VTK_CSM_IO_EXPORT __declspec( dllexport )
#else
#define VTK_CSM_IO_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_CSM_IO_EXPORT
#endif

#endif
