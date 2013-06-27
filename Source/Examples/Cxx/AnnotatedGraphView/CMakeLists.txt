#
# Add the executable
#

ADD_EXECUTABLE(AnnotatedGraphView AnnotatedGraphView.cxx)
TARGET_LINK_LIBRARIES(AnnotatedGraphView vtkcsmInfovis vtkcsmViews vtkViews)
