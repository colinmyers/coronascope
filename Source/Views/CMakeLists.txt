#
# Source files
#
# Here is where you can add the name of your local Views classes.
#

SET (Views_SRCS
  vtkAnnotatedGraphRepresentation.cxx
  vtkAnnotatedGraphView.cxx
  vtkInteractorStyleAnnotate2D.cxx
)

# --------------------------------------------------------------------------
# You probably do not need to modify anything below this line

# Create the vtkcsmViews C++ library.
ADD_LIBRARY (vtkcsmViews ${Views_SRCS})
TARGET_LINK_LIBRARIES(vtkcsmViews vtkcsmInfovis)

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES (vtkcsmViews vtkParallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES (vtkcsmViews vtkRendering)
  TARGET_LINK_LIBRARIES (vtkcsmViews vtkVolumeRendering)
  TARGET_LINK_LIBRARIES (vtkcsmViews vtkHybrid)
  TARGET_LINK_LIBRARIES (vtkcsmViews vtkWidgets)
ENDIF (VTK_USE_RENDERING)

# Create the vtkcsmViews Tcl library.
IF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)
  VTK_WRAP_TCL3 (vtkcsmViewsTCL ViewsTCL_SRCS "${Views_SRCS}" "")
  ADD_LIBRARY (vtkcsmViewsTCL ${ViewsTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkcsmViewsTCL vtkcsmCommonTCL vtkcsmViews)
ENDIF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)

# Create the vtkcsmViews Python library.
IF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)
  VTK_WRAP_PYTHON3 (vtkcsmViewsPython ViewsPython_SRCS "${Views_SRCS}")
  ADD_LIBRARY (vtkcsmViewsPythonD ${ViewsPython_SRCS})
  ADD_LIBRARY (vtkcsmViewsPython MODULE vtkcsmViewsPythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtkcsmViewsPythonD vtkcsmViews vtkcsmCommonPythonD)
  TARGET_LINK_LIBRARIES(vtkcsmViewsPython vtkcsmViewsPythonD)
ENDIF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)

# Create the vtkcsmViews Java library.
IF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
  VTK_WRAP_JAVA3 (vtkcsmViewsJava ViewsJava_SRCS "${Views_SRCS}")
  ADD_LIBRARY (vtkcsmViewsJava SHARED ${ViewsJava_SRCS})
  TARGET_LINK_LIBRARIES(vtkcsmViewsJava vtkcsmCommonJava vtkcsmViews)
ENDIF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
