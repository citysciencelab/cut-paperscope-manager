#########################################
#	QT
#########################################

TEMPLATE = app
TARGET = "PaperScope Manager"

QT += core core5compat gui widgets multimedia svgwidgets network websockets
#QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

CONFIG += c++17


#########################################
#	CODE
#########################################

SOURCES += \
    src/global/Settings.cpp \
    src/global/Api.cpp \
    src/global/Broadcast.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/paperscope/PaperScope.cpp \
    src/paperscope/capture/PSCalibrate.cpp \
    src/paperscope/capture/PSCapture.cpp \
    src/paperscope/detect/PSDetect.cpp \
    src/paperscope/detect/PSCandidate.cpp \
    src/paperscope/describe/PSDescribe.cpp \
    src/paperscope/describe/PSObject.cpp \
    src/ui/menu/MainMenu.cpp \
    src/ui/navi/MainNavi.cpp \
    src/ui/renderer/Renderer.cpp \
    src/ui/renderer/PreferencesTab.cpp \
    src/ui/renderer/DraggableRectItem.cpp \
    src/ui/renderer/mode/CameraModeConfig.cpp \
    src/ui/renderer/mode/PaperScopeModeConfig.cpp

HEADERS += \
    src/MainWindow.h \
    src/global/Settings.h \
    src/global/Api.h \
    src/global/Broadcast.h \
    src/paperscope/PaperScope.h \
    src/paperscope/PSTrackingMode.h \
    src/paperscope/PSViewMode.h \
    src/paperscope/capture/PSCalibrate.h \
    src/paperscope/capture/PSCapture.h \
    src/paperscope/detect/PSDetect.h \
    src/paperscope/detect/PSCandidate.h \
    src/paperscope/describe/PSDescribe.h \
    src/paperscope/describe/PSObject.h \
    src/ui/menu/MainMenu.h \
    src/ui/navi/MainNavi.h \
    src/ui/renderer/Renderer.h \
    src/ui/renderer/PreferencesTab.h \
    src/ui/renderer/DraggableRectItem.h \
    src/ui/renderer/mode/CameraModeConfig.h \
    src/ui/renderer/mode/PaperScopeModeConfig.h


#########################################
#	THIRDPARTY
#########################################

INCLUDEPATH += \
    thirdparty/tensorflow-lite/include \


#########################################
#	MAC
#########################################

macx {
    
    INCLUDEPATH += \
        thirdparty/opencv/include/mac

    ICON = resources/icon/PaperScopeIcon.icns

    # add support for pkg-config
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKG_CONFIG = /opt/homebrew/bin/pkg-config

    PKGCONFIG += opencv4

    LIBS += -L$$PWD/thirdparty/tensorflow-lite/lib/mac -ltensorflowlite
    DEPENDPATH += $$PWD/thirdparty/tensorflow-lite/lib/mac
    
    # add files to app bundle
    BundleFiles.path = Contents/MacOS
    BundleFiles.files += \
        $$PWD/thirdparty/tensorflow-lite/lib/mac/libtensorflowlite.dylib \
        #$$PWD/thirdparty/tensorflow-lite/lib/mac/libtensorflowlite_intel.dylib \
        $$PWD/resources/keras/shape-classifier_v4.tflite
    QMAKE_BUNDLE_DATA += BundleFiles
    QMAKE_INFO_PLIST = resources/os/mac/Info.plist
}


#########################################
#	WINDOWS
#########################################

CONFIG(debug, debug|release) {

    DDIR = debug

    # use precompiled OpenCV or use your own dependency
    OPENCV_LIBS = $$PWD/thirdparty/opencv/lib/win/debug
    OPENCV_BINS = $$PWD/thirdparty/opencv/bin/debug
    OPENCV_VERSION = 4100d
}


CONFIG(release, debug|release) {

    DDIR = release

    # use precompiled OpenCV or use your own dependency
    OPENCV_LIBS = $$PWD/thirdparty/opencv/lib/win/release
    OPENCV_BINS = $$PWD/thirdparty/opencv/bin/release
    OPENCV_VERSION = 4100
}


windows {

    INCLUDEPATH += \
        thirdparty/opencv/include/windows

    RC_ICONS = $$PWD/resources/icon/PaperScopeIcon.ico

    LIBS += -L$$OPENCV_LIBS \
        -lopencv_core$${OPENCV_VERSION} \
        -lopencv_aruco$${OPENCV_VERSION} \
        -lopencv_imgproc$${OPENCV_VERSION} \
        -lopencv_ximgproc$${OPENCV_VERSION} \
        -lopencv_videoio$${OPENCV_VERSION} \
        -lopencv_video$${OPENCV_VERSION} \
        -lopencv_objdetect$${OPENCV_VERSION} \
        -lopencv_calib3d$${OPENCV_VERSION} \
        -lopencv_flann$${OPENCV_VERSION} \
        -lopencv_features2d$${OPENCV_VERSION} \
        -lopencv_dnn$${OPENCV_VERSION} \
        -lopencv_imgcodecs$${OPENCV_VERSION}
    DEPENDPATH += $$PWD/thirdparty/opencv/bin/$$DDIR

    EXTRA_BINFILES += $$OPENCV_BINS/opencv_core$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_aruco$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_imgproc$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_ximgproc$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_videoio$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_video$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_objdetect$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_calib3d$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_flann$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_features2d$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_dnn$${OPENCV_VERSION}.dll
    EXTRA_BINFILES += $$OPENCV_BINS/opencv_imgcodecs$${OPENCV_VERSION}.dll
   
    LIBS += -L$$PWD/thirdparty/tensorflow-lite/lib/win -ltensorflowlite.dll.if
    DEPENDPATH += $$PWD/thirdparty/tensorflow-lite/lib/win

    EXTRA_BINFILES += $$PWD/thirdparty/tensorflow-lite/bin/tensorflowlite.dll
    EXTRA_BINFILES += $$PWD/thirdparty/tbb/lib/tbb12.dll
    EXTRA_BINFILES += $$PWD/resources/keras/shape-classifier_v4.tflite
    
    for(FILE,EXTRA_BINFILES){
        win32:FILE ~= s,/,\\,g
        win32:DDIR ~= s,/,\\,g
        QMAKE_POST_LINK += $$quote(cp $${FILE} $$DDIR $$escape_expand(\\n\\t))
    }
}


#########################################
#	DEPLOYMENT
#########################################

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += resources/resources.qrc
