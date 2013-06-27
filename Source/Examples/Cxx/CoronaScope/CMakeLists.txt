FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
 
# support for out-of-source build
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Set your files and resources here
SET(CoronaScopeSrcs
    MainApp.cxx
    MainWindow.cxx
    About.cxx
    Preferences.cxx
    Pipeline.cxx
    ScreenShot.cxx
    BrewerColorTableModel.cxx
    BrewerColorSet.cxx
    BrewerColorDelegate.cxx
    )
    
SET(CoronaScopeUI
    About.ui
    Preferences.ui
    ScreenShot.ui
    MainWindow.ui
    )
    
SET(CoronaScopeHeaders
    MainWindow.h
    About.h
    Preferences.h
    ScreenShot.h
    BrewerColorDelegate.h
    )
    
SET(CoronaScopeResources
    application.qrc
    )

QT4_WRAP_UI(UISrcs ${CoronaScopeUI})
QT4_ADD_RESOURCES(RCCSrcs ${CoronaScopeResources})
QT4_WRAP_CPP(MOCSrcs ${CoronaScopeHeaders})
 
#SOURCE_GROUP("Resources" FILES
#  ${CoronaScopeUI}
#)
 
#SOURCE_GROUP("Generated" FILES
#  ${UISrcs}
#  ${MOCSrcs}
#  ${RCCSrcs}
#)
 
ADD_EXECUTABLE( CoronaScope ${CoronaScopeSrcs} ${UISrcs} ${MOCSrcs} ${RCCSrcs})

TARGET_LINK_LIBRARIES(CoronaScope ${OGDF_LIBRARIES} QVTK vtkcsmGUISupport vtkcsmInfovis
                        vtkcsmViews vtkcsmIO vtkcsmWidgets)

