#
# Source files
#
# Here is where you can add the name of your local IO classes.
#

SET (IO_SRCS
    vtkAnnotationLayersSerializer.cxx
    vtkPVXMLElement.cxx
    vtkPVXMLParser.cxx
    vtkSelectionSerializer.cxx
)

# --------------------------------------------------------------------------
# You probably do not need to modify anything below this line

# Create the vtkcsmIO C++ library.
ADD_LIBRARY (vtkcsmIO ${IO_SRCS})
#TARGET_LINK_LIBRARIES(vtkcsmIO vtkcsmGraphics)

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES (vtkcsmIO vtkParallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES (vtkcsmIO vtkRendering)
  TARGET_LINK_LIBRARIES (vtkcsmIO vtkVolumeRendering)
  TARGET_LINK_LIBRARIES (vtkcsmIO vtkHybrid)
  TARGET_LINK_LIBRARIES (vtkcsmIO vtkIO)
ENDIF (VTK_USE_RENDERING)

# Create the vtkcsmIO Tcl library.
IF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)
  VTK_WRAP_TCL3 (vtkcsmIOTCL IOTCL_SRCS "${IO_SRCS}" "")
  ADD_LIBRARY (vtkcsmIOTCL ${IOTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkcsmIOTCL vtkcsmCommonTCL vtkcsmIO)
ENDIF (VTK_WRAP_TCL AND VTK_MY_WRAP_TCL)

# Create the vtkcsmIO Python library.
IF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)
  VTK_WRAP_PYTHON3 (vtkcsmIOPython IOPython_SRCS "${IO_SRCS}")
  ADD_LIBRARY (vtkcsmIOPythonD ${IOPython_SRCS})
  ADD_LIBRARY (vtkcsmIOPython MODULE vtkcsmIOPythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtkcsmIOPythonD vtkcsmIO vtkcsmCommonPythonD)
  TARGET_LINK_LIBRARIES(vtkcsmIOPython vtkcsmIOPythonD)
ENDIF (VTK_WRAP_PYTHON AND VTK_MY_WRAP_PYTHON)

# Create the vtkcsmIO Java library.
IF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
  VTK_WRAP_JAVA3 (vtkcsmIOJava IOJava_SRCS "${IO_SRCS}")
  ADD_LIBRARY (vtkcsmIOJava SHARED ${IOJava_SRCS})
  TARGET_LINK_LIBRARIES(vtkcsmIOJava vtkcsmCommonJava vtkcsmIO)
ENDIF (VTK_WRAP_JAVA AND VTK_MY_WRAP_JAVA)
