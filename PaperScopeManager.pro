#########################################
#	QT
#########################################

TEMPLATE = app
TARGET = "PaperScope Manager"

QT += core core5compat gui widgets multimedia svgwidgets network websockets

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
    thirdparty/opencv/include


#########################################
#	MAC
#########################################

macx {
    ICON = resources/icon/PaperScopeIcon.icns

    # add support for pkg-config
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKG_CONFIG = /opt/homebrew/bin/pkg-config

    PKGCONFIG += opencv4

    QMAKE_INFO_PLIST = resources/os/mac/Info.plist
    LIBS += -L$$PWD/thirdparty/tensorflow-lite/lib/mac -ltensorflowlite
    DEPENDPATH += $$PWD/thirdparty/tensorflow-lite/lib/mac
    
    # add files to app bundle
    BundleFiles.path = Contents/MacOS
    BundleFiles.files += \
        $$PWD/thirdparty/tensorflow-lite/lib/mac/libtensorflowlite.dylib \
        #$$PWD/thirdparty/tensorflow-lite/lib/mac/libtensorflowlite_intel.dylib \
        $$PWD/resources/keras/shape-classifier_v4.tflite
    QMAKE_BUNDLE_DATA += BundleFiles
}


#########################################
#	WINDOWS
#########################################

CONFIG(debug, debug|release) {

    windows {
        LIBS += -L$$PWD/thirdparty/opencv/lib/win/debug \
            -lopencv_core4100d \
            -lopencv_aruco4100d \
            -lopencv_imgproc4100d \
            -lopencv_ximgproc4100d \
            -lopencv_videoio4100d \
            -lopencv_objdetect4100d \
            -lopencv_calib3d4100d \
            -lopencv_flann4100d \
            -lopencv_features2d4100d \
            -lopencv_dnn4100d \
            -lopencv_imgcodecs4100d
        DEPENDPATH += $$PWD/thirdparty/opencv/bin/debug
    }
}


CONFIG(release, debug|release) {

    windows {
        LIBS += -L$$PWD/thirdparty/opencv/lib/win/release \
            -lopencv_core4100 \
            -lopencv_aruco4100 \
            -lopencv_imgproc4100 \
            -lopencv_ximgproc4100 \
            -lopencv_videoio4100 \
            -lopencv_objdetect4100 \
            -lopencv_calib3d4100 \
            -lopencv_flann4100 \
            -lopencv_features2d4100 \
            -lopencv_dnn4100 \
            -lopencv_imgcodecs4100
        DEPENDPATH += $$PWD/thirdparty/opencv/bin/release
    }
}

windows {
   
    LIBS += -L$$PWD/thirdparty/tensorflow-lite/lib/win -ltensorflowlite.dll.if
    DEPENDPATH += $$PWD/thirdparty/tensorflow-lite/lib/win
}


#########################################
#	DEPLOYMENT
#########################################

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += resources/resources.qrc
