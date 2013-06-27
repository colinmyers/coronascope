#
# Source files
#
# Here is where you can add the name of your local Infovis classes.
#


set (Infovis_SRCS
  vtkFlightMapFilter.cxx
  vtkMySpanTreeLayoutStrategy.cxx
)

if(OGDF_ON)
   set (Infovis_SRCS ${Infovis_SRCS}
   vtkOGDFLayoutStrategy.cxx
   )
endif()

# --------------------------------------------------------------------------
# You probably do not need to modify anything below this line

# Create the vtkcsmInfovis C++ library.
ADD_LIBRARY (vtkcsmInfovis ${Infovis_SRCS})

if(OGDF_ON)
  TARGET_LINK_LIBRARIES(vtkcsmInfovis ${OGDF_LIBRARIES})
endif()

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES (vtkcsmInfovis vtkParallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES (vtkcsmInfovis vtkRendering)
  TARGET_LINK_LIBRARIES (vtkcsmInfovis vtkVolumeRendering)
  TARGET_LINK_LIBRARIES (vtkcsmInfovis vtkHybrid)
  TARGET_LINK_LIBRARIES (vtkcsmInfovis vtkWidgets)
ENDIF (VTK_USE_RENDERING)

# Create the vtkcsmInfovis Tcl library.
IF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)
  VTK_WRAP_TCL3 (vtkcsmInfovisTCL InfovisTCL_SRCS "${Infovis_SRCS}" "")
  ADD_LIBRARY (vtkcsmInfovisTCL ${InfovisTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkcsmInfovisTCL vtkcsmCommonTCL vtkcsmInfovis)
ENDIF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)

# Create the vtkcsmInfovis Python library.
IF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)
  VTK_WRAP_PYTHON3 (vtkcsmInfovisPython InfovisPython_SRCS "${Infovis_SRCS}")
  ADD_LIBRARY (vtkcsmInfovisPythonD ${InfovisPython_SRCS})
  ADD_LIBRARY (vtkcsmInfovisPython MODULE vtkcsmInfovisPythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtkcsmInfovisPythonD vtkcsmInfovis vtkcsmCommonPythonD)
  TARGET_LINK_LIBRARIES(vtkcsmInfovisPython vtkcsmInfovisPythonD)
ENDIF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)

# Create the vtkcsmInfovis Java library.
IF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
  VTK_WRAP_JAVA3 (vtkcsmInfovisJava InfovisJava_SRCS "${Infovis_SRCS}")
  ADD_LIBRARY (vtkcsmInfovisJava SHARED ${InfovisJava_SRCS})
  TARGET_LINK_LIBRARIES(vtkcsmInfovisJava vtkcsmCommonJava vtkcsmInfovis)
ENDIF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
