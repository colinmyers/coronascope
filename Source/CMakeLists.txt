CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

#
# Set the project name.
#

PROJECT (VTK_MY)

#
# Load CMake commands that you probably should not modify.
#

INCLUDE (${VTK_MY_SOURCE_DIR}/CMakeOptions.cmake)

#
# Here is where you can list the sub-directories holding your local
# classes. Sorting classes by 'package' type like VTK does (Common,
# Rendering, Filtering, Imaging, IO, etc.) is a good thing and prevents
# numerous dependencies problems.
#

SUBDIRS (
    GUISupport
    Infovis
    IO
    Utilities
    Views
    Widgets
)

# Set up conditional compilation of OGDF dependencies, e.g. vtkOGDFLayoutStrategy
OPTION(OGDF_ON "Build OGDF layout algorithms" ON)
if (OGDF_ON)
  find_package(OGDF REQUIRED)
  INCLUDE_DIRECTORIES(
                     ${OGDF_INCLUDE_DIRS}
  )  
  ADD_DEFINITIONS(-D__USE_OGDF__)
endif()

INCLUDE_DIRECTORIES (
                     ${VTK_MY_SOURCE_DIR}/GUISupport
                     ${VTK_MY_SOURCE_DIR}/Infovis
                     ${VTK_MY_SOURCE_DIR}/IO
                     ${VTK_MY_SOURCE_DIR}/Views
                     ${VTK_MY_SOURCE_DIR}/Widgets
)

#
# Build examples too ?
#

OPTION(BUILD_EXAMPLES "Build examples." ON)
IF (BUILD_EXAMPLES)
  SUBDIRS(Examples)
ENDIF (BUILD_EXAMPLES)

#
# Common configuration settings
#
# Do not worry about this one.
#

CONFIGURE_FILE(
  ${VTK_MY_SOURCE_DIR}/vtkcsmConfigure.h.in
  ${VTK_MY_BINARY_DIR}/vtkcsmConfigure.h
)

INCLUDE_DIRECTORIES(${VTK_MY_BINARY_DIR})
