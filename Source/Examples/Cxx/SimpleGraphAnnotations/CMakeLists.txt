#
# Add the executable
#

ADD_EXECUTABLE(SimpleGraphAnnotations SimpleGraphAnnotations.cxx)
TARGET_LINK_LIBRARIES(SimpleGraphAnnotations vtkcsmInfovis vtkcsmViews vtkViews)
