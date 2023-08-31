cmake_minimum_required(VERSION 3.5)
project(ads_demo VERSION ${VERSION_SHORT})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.5 COMPONENTS Core Gui Widgets Quick QuickWidgets REQUIRED)
if (WIN32 AND QT_VERSION_MAJOR LESS 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS AxContainer REQUIRED)
endif ()
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(VersaDebugAssistantSOURCE
        main.cpp
        MainWindow.cpp
        mainwindow.ui
        StatusDialog.cpp
        StatusDialog.ui
        ImageViewer.cpp
        RenderWidget.cpp
        demo.qrc
        )

add_executable(VersaDebugAssistant WIN32 ${VersaDebugAssistantSOURCE})

target_include_directories(VersaDebugAssistant PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

target_link_libraries(VersaDebugAssistant PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickWidgets)

if (WIN32 AND QT_VERSION_MAJOR LESS 6)
    target_link_libraries(VersaDebugAssistant PUBLIC
            Qt${QT_VERSION_MAJOR}::AxContainer)
endif ()

target_link_libraries(VersaDebugAssistant PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)

set_target_properties(VersaDebugAssistant PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        VERSION ${VERSION_SHORT}
        EXPORT_NAME "Qt Advanced Docking System Demo"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
        )
#if(BUILD_STATIC)
#    target_compile_definitions(VersaDebugAssistant PRIVATE ADS_STATIC)
#endif()

