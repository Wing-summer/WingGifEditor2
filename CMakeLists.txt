cmake_minimum_required(VERSION 3.16)

project(WingGifEditor2 LANGUAGES CXX)

set(PROJECT_VERSION 1.0.1)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent LinguistTools)

add_definitions(-DWINGGIF_VERSION="${PROJECT_VERSION}"
                -DAPP_ORG="WingCloudStudio" -DAPP_NAME="${PROJECT_NAME}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WINGGIF_VERSION" ${PROJECT_VERSION})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/QT_VERSION" ${QT_VERSION_MAJOR})

option(WINGGIF_USE_FRAMELESS "Use borderless windows to ensure UI uniformity"
       TRUE)

add_subdirectory(3rdparty/giflib)

find_package(OpenMP REQUIRED)

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
endif()

if(WINGGIF_USE_FRAMELESS)
    option(QWINDOWKIT_BUILD_STATIC "Build static libraries" TRUE)
    option(QWINDOWKIT_INSTALL "Install library" OFF)
    add_subdirectory(3rdparty/qwindowkit)
    add_definitions(-DWINGHEX_USE_FRAMELESS)

    set(WIDGET_FRAME_SRC
        src/widgetframe/windowbar.cpp
        src/widgetframe/windowbar.h
        src/widgetframe/windowbar_p.h
        src/widgetframe/windowbutton.cpp
        src/widgetframe/windowbutton.h
        src/widgetframe/windowbutton_p.h
        src/class/framelesshelper.h
        src/class/framelesshelper.cpp)
else()
    set(WIDGET_FRAME_SRC)
endif()

set(RIBBON_SRC
    3rdparty/QWingRibbon/ribbon.cpp
    3rdparty/QWingRibbon/ribbon.h
    3rdparty/QWingRibbon/ribbonbuttongroup.cpp
    3rdparty/QWingRibbon/ribbonbuttongroup.h
    3rdparty/QWingRibbon/ribbonbuttongroup.ui
    3rdparty/QWingRibbon/ribbontabcontent.cpp
    3rdparty/QWingRibbon/ribbontabcontent.h
    3rdparty/QWingRibbon/ribbontabcontent.ui)

set(QPATHEDIT_SRC 3rdparty/QPathEdit/qpathedit.cpp
                  3rdparty/QPathEdit/qpathedit.h)

set(PROJECT_SOURCES
    src/main.cpp
    src/dialog/mainwindow.cpp
    src/dialog/mainwindow.h
    src/dialog/framelessdialogbase.cpp
    src/dialog/framelessdialogbase.h
    src/dialog/framelessmainwindow.cpp
    src/dialog/framelessmainwindow.h
    src/dialog/reduceframedialog.cpp
    src/dialog/reduceframedialog.h
    src/dialog/createreversedialog.cpp
    src/dialog/createreversedialog.h
    src/dialog/exportdialog.cpp
    src/dialog/exportdialog.h
    src/dialog/newdialog.cpp
    src/dialog/newdialog.h
    src/dialog/scalegifdialog.cpp
    src/dialog/scalegifdialog.h
    src/dialog/cropgifdialog.cpp
    src/dialog/cropgifdialog.h
    src/dialog/aboutsoftwaredialog.cpp
    src/dialog/aboutsoftwaredialog.h
    src/dialog/aboutsoftwaredialog.ui
    src/dialog/fileinfodialog.cpp
    src/dialog/fileinfodialog.h
    src/dialog/gifsavedialog.h
    src/dialog/gifsavedialog.cpp
    src/dialog/logdialog.h
    src/dialog/logdialog.cpp
    src/class/appmanager.cpp
    src/class/appmanager.h
    src/class/clipboardhelper.cpp
    src/class/clipboardhelper.h
    src/class/languagemanager.cpp
    src/class/languagemanager.h
    src/class/gifreader.h
    src/class/gifreader.cpp
    src/class/gifwriter.h
    src/class/gifwriter.cpp
    src/class/gifcontentmodel.h
    src/class/gifcontentmodel.cpp
    src/class/picturedelegate.h
    src/class/picturedelegate.cpp
    src/class/qkeysequences.h
    src/class/qkeysequences.cpp
    src/class/recentfilemanager.cpp
    src/class/recentfilemanager.h
    src/class/setting.h
    src/class/eventfilter.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/playgifmanager.cpp
    src/class/playgifmanager.h
    src/class/winginputdialog.cpp
    src/class/winginputdialog.h
    src/class/wingmessagebox.cpp
    src/class/wingmessagebox.h
    src/class/wingprogressdialog.cpp
    src/class/wingprogressdialog.h
    src/class/settingmanager.cpp
    src/class/settingmanager.h
    src/class/logger.cpp
    src/class/logger.h
    src/class/waitingloop.h
    src/class/waitingloop.cpp
    src/control/gifcontentgallery.h
    src/control/gifcontentgallery.cpp
    src/control/toast.cpp
    src/control/toast.h
    src/control/imagecropper/imagecropper_e.h
    src/control/imagecropper/imagecropper_p.h
    src/control/imagecropper/imagecropper.cpp
    src/control/imagecropper/imagecropper.h
    src/control/gifeditor.cpp
    src/control/gifeditor.h
    src/control/gifeditorscene.cpp
    src/control/gifeditorscene.h
    src/command/cropimagecommand.cpp
    src/command/cropimagecommand.h
    src/command/delayframecommand.cpp
    src/command/delayframecommand.h
    src/command/delframedircommand.cpp
    src/command/delframedircommand.h
    src/command/flipframecommand.cpp
    src/command/flipframecommand.h
    src/command/insertframecommand.cpp
    src/command/insertframecommand.h
    src/command/moveframecommand.cpp
    src/command/moveframecommand.h
    src/command/reduceframecommand.cpp
    src/command/reduceframecommand.h
    src/command/removeframecommand.cpp
    src/command/removeframecommand.h
    src/command/replaceframecommand.cpp
    src/command/replaceframecommand.h
    src/command/reverseframecommand.cpp
    src/command/reverseframecommand.h
    src/command/rotateframecommand.cpp
    src/command/rotateframecommand.h
    src/command/scaleframecommand.cpp
    src/command/scaleframecommand.h
    src/settings/pluginsettingdialog.cpp
    src/settings/pluginsettingdialog.h
    src/settings/pluginsettingdialog.ui
    src/settings/editorsettingdialog.cpp
    src/settings/editorsettingdialog.h
    src/settings/editorsettingdialog.ui
    src/plugin/iwingplugin.h
    src/plugin/pluginsystem.h
    src/plugin/pluginsystem.cpp
    ${WIDGET_FRAME_SRC}
    ${QPATHEDIT_SRC}
    ${RIBBON_SRC}
    src/utilities.h
    src/dbghelper.h
    theme/breeze.qrc
    resources.qrc)

include_directories("${CMAKE_SOURCE_DIR}/src")

set(TRANSLATION_FILES "lang/${PROJECT_NAME}_zh_CN.ts")
set(QM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lang")
file(MAKE_DIRECTORY ${QM_OUTPUT_DIR})

set_source_files_properties(${TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION
                                                            "lang")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src
                           ${TRANSLATION_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src
                           ${TRANSLATION_FILES})
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(WIN32)
        set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
        qt_add_executable(WingGifEditor2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                          ${QM_FILES} ${app_icon_resource_windows})
    else()
        qt_add_executable(WingGifEditor2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                          ${QM_FILES})
    endif()

    # Define target properties for Android with Qt 6 as: set_property(TARGET
    # WingGifEditor2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    # ${CMAKE_CURRENT_SOURCE_DIR}/android) For more information, see
    # https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        message(FATAL_ERROR "Not supported for Android!")
    else()
        if(WIN32)
            set(app_icon_resource_windows
                "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
            add_executable(WingGifEditor2 ${PROJECT_SOURCES} ${QM_FILES}
                                          ${app_icon_resource_windows})
        else()
            add_executable(WingGifEditor2 ${PROJECT_SOURCES} ${QM_FILES})
        endif()
    endif()
endif()

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
                                  "${CMAKE_CURRENT_SOURCE_DIR}")

if(WINGGIF_USE_FRAMELESS)
    target_link_libraries(
        WingGifEditor2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent gif_object_lib
                               QWKCore QWKWidgets OpenMP::OpenMP_CXX)
else()
    target_link_libraries(
        WingGifEditor2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent gif_object_lib
                               OpenMP::OpenMP_CXX)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1. If
# you are developing for iOS or macOS you should consider setting an explicit,
# fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER
                         com.example.WingGifEditor2)
endif()
set_target_properties(
    WingGifEditor2
    PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
               ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
               TRUE WIN32_EXECUTABLE
               TRUE)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WingGifEditor2)
endif()
