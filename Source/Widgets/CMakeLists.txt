#
# Source files
#
# Here is where you can add the name of your local Widgets classes.
#

SET (Widgets_SRCS
    vtkOffScreenRepresentationImpl.cxx
    vtkOffScreenRepresentation.cxx
    vtkOffScreenWidget.cxx
)

# --------------------------------------------------------------------------
# You probably do not need to modify anything below this line

# Create the vtkcsmWidgets C++ library.
ADD_LIBRARY (vtkcsmWidgets ${Widgets_SRCS})
TARGET_LINK_LIBRARIES(vtkcsmWidgets )

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES (vtkcsmWidgets vtkParallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES (vtkcsmWidgets vtkRendering)
  TARGET_LINK_LIBRARIES (vtkcsmWidgets vtkVolumeRendering)
  TARGET_LINK_LIBRARIES (vtkcsmWidgets vtkHybrid)
  TARGET_LINK_LIBRARIES (vtkcsmWidgets vtkWidgets)
ENDIF (VTK_USE_RENDERING)

# Create the vtkcsmWidgets Tcl library.
IF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)
  VTK_WRAP_TCL3 (vtkcsmWidgetsTCL WidgetsTCL_SRCS "${Widgets_SRCS}" "")
  ADD_LIBRARY (vtkcsmWidgetsTCL ${WidgetsTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkcsmWidgetsTCL vtkcsmCommonTCL vtkcsmWidgets)
ENDIF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)

# Create the vtkcsmWidgets Python library.
IF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)
  VTK_WRAP_PYTHON3 (vtkcsmWidgetsPython WidgetsPython_SRCS "${Widgets_SRCS}")
  ADD_LIBRARY (vtkcsmWidgetsPythonD ${WidgetsPython_SRCS})
  ADD_LIBRARY (vtkcsmWidgetsPython MODULE vtkcsmWidgetsPythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtkcsmWidgetsPythonD vtkcsmWidgets vtkcsmCommonPythonD)
  TARGET_LINK_LIBRARIES(vtkcsmWidgetsPython vtkcsmWidgetsPythonD)
ENDIF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)

# Create the vtkcsmWidgets Java library.
IF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
  VTK_WRAP_JAVA3 (vtkcsmWidgetsJava WidgetsJava_SRCS "${Widgets_SRCS}")
  ADD_LIBRARY (vtkcsmWidgetsJava SHARED ${WidgetsJava_SRCS})
  TARGET_LINK_LIBRARIES(vtkcsmWidgetsJava vtkcsmCommonJava vtkcsmWidgets)
ENDIF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
