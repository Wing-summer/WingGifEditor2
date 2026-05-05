cmake_minimum_required(VERSION 3.20)

project(WingGifEditor2 LANGUAGES CXX)

set(PROJECT_VERSION 1.1.1)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent LinguistTools)

if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    set(QT_CREATOR_SKIP_MAINTENANCE_TOOL_PROVIDER ON)
    set(QT_NO_PRIVATE_MODULE_WARNING ON)
    find_package(Qt6 REQUIRED GuiPrivate CorePrivate WidgetsPrivate)
endif()

add_definitions(-DWINGGIF_VERSION="${PROJECT_VERSION}"
                -DAPP_ORG="WingCloudStudio" -DAPP_NAME="${PROJECT_NAME}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WINGGIF_VERSION" ${PROJECT_VERSION})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/QT_VERSION" ${QT_VERSION_MAJOR})

option(WINGGIF_USE_FRAMELESS "Use borderless windows to ensure UI uniformity"
       TRUE)

add_subdirectory(3rdparty/giflib)
add_subdirectory(3rdparty/angelscript/sdk/angelscript/projects/cmake)

include_directories(3rdp)

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

set(DIALOG_SRC
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
    src/dialog/cropgifdialog.cpp
    src/dialog/cropgifdialog.h
    src/dialog/newdialog.cpp
    src/dialog/newdialog.h
    src/dialog/scalegifdialog.cpp
    src/dialog/scalegifdialog.h
    src/dialog/aboutsoftwaredialog.cpp
    src/dialog/aboutsoftwaredialog.h
    src/dialog/aboutsoftwaredialog.ui
    src/dialog/colorpickerdialog.cpp
    src/dialog/colorpickerdialog.h
    src/dialog/colorpickerdialog.ui
    src/dialog/fileinfodialog.cpp
    src/dialog/fileinfodialog.h
    src/dialog/gifsavedialog.h
    src/dialog/gifsavedialog.cpp)

set(CLS_SRC
    src/class/appmanager.cpp
    src/class/appmanager.h
    src/class/clipboardhelper.cpp
    src/class/clipboardhelper.h
    src/class/languagemanager.cpp
    src/class/languagemanager.h
    src/class/giffile.h
    src/class/giffile.cpp
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
    src/class/settings.h
    src/class/eventfilter.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/playgifmanager.cpp
    src/class/playgifmanager.h
    src/class/winginputdialog.cpp
    src/class/winginputdialog.h
    src/class/wingmessagebox.cpp
    src/class/wingmessagebox.h
    src/class/wingfiledialog.cpp
    src/class/wingfiledialog.h
    src/class/wingprogressdialog.cpp
    src/class/wingprogressdialog.h
    src/class/settingmanager.cpp
    src/class/settingmanager.h
    src/class/scriptmanager.cpp
    src/class/scriptmanager.h
    src/class/compositeiconengine.cpp
    src/class/compositeiconengine.h
    src/class/qtlockedfile.h
    src/class/qtlockedfile.cpp
    src/class/angelscripthelper.h
    src/class/scriptmachine.cpp
    src/class/scriptmachine.h
    src/class/gifframe.h
    src/class/gifframe.cpp
    src/class/asgifimage.h
    src/class/asgifimage.cpp
    src/class/waitingloop.h
    src/class/waitingloop.cpp)

set(CONTORL_SRC
    src/control/gifcontentgallery.h
    src/control/gifcontentgallery.cpp
    src/control/toast.cpp
    src/control/toast.h
    src/control/huecolorpickerslider.cpp
    src/control/huecolorpickerslider.h
    src/control/qcolorpicker_slider.cpp
    src/control/qcolorpicker_slider.hpp
    src/control/aspectratiopixmaplabel.h
    src/control/aspectratiopixmaplabel.cpp
    src/control/imagecropper/imagecropper_e.h
    src/control/imagecropper/imagecropper_p.h
    src/control/imagecropper/imagecropper.cpp
    src/control/imagecropper/imagecropper.h
    src/control/gifeditor.cpp
    src/control/gifeditor.h
    src/control/gifeditorscene.cpp
    src/control/gifeditorscene.h)

set(CMD_SRC
    src/command/undocommand.h
    src/command/undocommand.cpp
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
    src/command/scaleframecommand.h)

set(SET_SRC
    src/settings/editorsettingdialog.cpp src/settings/editorsettingdialog.h
    src/settings/editorsettingdialog.ui)

set(ANGEL_SCRIPT_ADDON_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/angelscript/sdk/add_on")
set(ANGEL_SCRIPT_ADDON
    ${ANGEL_SCRIPT_ADDON_ROOT}/autowrapper/aswrappedcall.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptgrid/scriptgrid.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptgrid/scriptgrid.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthandle/scripthandle.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthandle/scripthandle.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthelper/scripthelper.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthelper/scripthelper.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmath.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmath.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmathcomplex.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmathcomplex.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptbuilder/scriptbuilder.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptbuilder/scriptbuilder.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/weakref/weakref.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/weakref/weakref.h)

set(SCRIPT_ADDON_SRC
    src/scriptaddon/contextmgr.cpp
    src/scriptaddon/contextmgr.h
    src/scriptaddon/scriptrunable.h
    src/scriptaddon/scriptrunable.cpp
    src/scriptaddon/scriptqstring.h
    src/scriptaddon/scriptqstring.cpp
    src/scriptaddon/scriptqdictionary.h
    src/scriptaddon/scriptqdictionary.cpp
    src/scriptaddon/scriptregex.h
    src/scriptaddon/scriptregex.cpp
    src/scriptaddon/scriptcolor.h
    src/scriptaddon/scriptcolor.cpp
    src/scriptaddon/qtgeometrybinding.cpp
    src/scriptaddon/qtgeometrybinding.h
    src/scriptaddon/scriptjson.h
    src/scriptaddon/scriptjson.cpp
    src/scriptaddon/scriptoptbox.h
    src/scriptaddon/scriptoptbox.cpp
    src/scriptaddon/scriptany.cpp
    src/scriptaddon/scriptany.h)

set(PROJECT_SOURCES
    src/main.cpp
    ${DIALOG_SRC}
    ${CLS_SRC}
    ${CONTORL_SRC}
    ${SET_SRC}
    ${ANGEL_SCRIPT_ADDON}
    ${SCRIPT_ADDON_SRC}
    ${CMD_SRC}
    ${WIDGET_FRAME_SRC}
    ${RIBBON_SRC}
    src/utilities.h
    src/predefgen.h
    src/dbghelper.h
    resources.qrc
    src/define.h
    theme/breeze.qrc)

set(TRANSLATION_FILES
    "lang/${PROJECT_NAME}_en_US.ts" "lang/${PROJECT_NAME}_zh_CN.ts"
    "lang/${PROJECT_NAME}_zh_TW.ts")
set(QM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lang")
file(MAKE_DIRECTORY ${QM_OUTPUT_DIR})

set_source_files_properties(${TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION
                                                            "lang")

qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TRANSLATION_FILES}
                       OPTIONS -no-obsolete)

if(WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
    qt_add_executable(WingGifEditor2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                      ${QM_FILES} ${app_icon_resource_windows})
else()
    qt_add_executable(WingGifEditor2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                      ${QM_FILES})
endif()

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
                                  "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(WingGifEditor2 PRIVATE AS_PROCESS_METADATA=0)

if(WINGGIF_USE_FRAMELESS)
    target_link_libraries(
        WingGifEditor2 PRIVATE Qt6::Widgets Qt6::Concurrent gif_object_lib
                               QWKCore QWKWidgets angelscript)
else()
    target_link_libraries(WingGifEditor2 PRIVATE Qt6::Widgets Qt6::Concurrent
                                                 gif_object_lib angelscript)
endif()

if(Qt6_VERSION VERSION_GREATER_EQUAL 6.7.0)
    target_compile_definitions(
        WingGifEditor2
        PRIVATE QT_NO_CONTEXTLESS_CONNECT QT_RESTRICTED_CAST_FROM_ASCII
                QT_NO_CAST_TO_ASCII)
endif()

# for QString contraction optimization
target_compile_definitions(
    WingGifEditor2
    PRIVATE
    PRIVATE QT_USE_QSTRINGBUILDER)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1. If
# you are developing for iOS or macOS you should consider setting an explicit,
# fixed bundle identifier manually though.
if(Qt6_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER
                         com.wingsummer.WingGifEditor2)
endif()

set_target_properties(
    WingGifEditor2
    PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
               ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
               TRUE WIN32_EXECUTABLE
               TRUE)

qt_finalize_executable(WingGifEditor2)
