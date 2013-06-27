#
# Source files
#
# Here is where you can add the name of your local GUISupport classes.
#

SET (GUISupport_SRCS
  vtkMyQtAnnotationLayersModelAdapter.cxx
)
SET (GUISupportHeaders
  vtkMyQtAnnotationLayersModelAdapter.h
)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})  # include UseQt4.cmake
QT4_WRAP_CPP(MOCSrcs ${GUISupportHeaders})

# --------------------------------------------------------------------------
# You probably do not need to modify anything below this line

# Create the vtkcsmGUISupport C++ library.
ADD_LIBRARY (vtkcsmGUISupport ${GUISupport_SRCS} ${MOCSrcs})
#TARGET_LINK_LIBRARIES(vtkcsmGUISupport )

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES (vtkcsmGUISupport vtkParallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES (vtkcsmGUISupport vtkRendering)
  TARGET_LINK_LIBRARIES (vtkcsmGUISupport vtkVolumeRendering)
  TARGET_LINK_LIBRARIES (vtkcsmGUISupport vtkHybrid)
  TARGET_LINK_LIBRARIES (vtkcsmGUISupport vtkWidgets)
ENDIF (VTK_USE_RENDERING)

# Create the vtkcsmGUISupport Tcl library.
IF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)
  VTK_WRAP_TCL3 (vtkcsmGUISupportTCL GUISupportTCL_SRCS "${GUISupport_SRCS}" "")
  ADD_LIBRARY (vtkcsmGUISupportTCL ${GUISupportTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkcsmGUISupportTCL vtkcsmCommonTCL vtkcsmGUISupport)
ENDIF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)

# Create the vtkcsmGUISupport Python library.
IF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)
  VTK_WRAP_PYTHON3 (vtkcsmGUISupportPython GUISupportPython_SRCS "${GUISupport_SRCS}")
  ADD_LIBRARY (vtkcsmGUISupportPythonD ${GUISupportPython_SRCS})
  ADD_LIBRARY (vtkcsmGUISupportPython MODULE vtkcsmGUISupportPythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtkcsmGUISupportPythonD vtkcsmGUISupport vtkcsmCommonPythonD)
  TARGET_LINK_LIBRARIES(vtkcsmGUISupportPython vtkcsmGUISupportPythonD)
ENDIF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)

# Create the vtkcsmGUISupport Java library.
IF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
  VTK_WRAP_JAVA3 (vtkcsmGUISupportJava GUISupportJava_SRCS "${GUISupport_SRCS}")
  ADD_LIBRARY (vtkcsmGUISupportJava SHARED ${GUISupportJava_SRCS})
  TARGET_LINK_LIBRARIES(vtkcsmGUISupportJava vtkcsmCommonJava vtkcsmGUISupport)
ENDIF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
