cmake_minimum_required(VERSION 3.16)
project(TreeGraphStudio VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Multimedia MultimediaWidgets)

add_executable(TreeGraphStudio 
    main.cpp
    src/ui/MainWindow.cpp
    src/ui/GraphCanvas.cpp
    src/ui/HelpWindow.cpp
    src/study/StudyWindow.cpp
    src/study/DocumentViewer.cpp
    src/study/GraphPracticeWidget.cpp
    src/study/VideoPlayerWidget.cpp
    resources.qrc
)

if(WIN32)
    target_sources(TreeGraphStudio PRIVATE resources/windows/app.rc)
endif()

target_link_libraries(TreeGraphStudio PRIVATE 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Gui 
    Qt5::Multimedia 
    Qt5::MultimediaWidgets
)

target_include_directories(TreeGraphStudio PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/study
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# MSVC(Visual Studio): 源码为 UTF-8 无 BOM，/utf-8 避免界面中文乱码与 C4819 警告；对 MinGW 无影响
if(MSVC)
    target_compile_options(TreeGraphStudio PRIVATE /utf-8)
    # MinGW 默认提供 M_PI 宏；MSVC 需先定义 _USE_MATH_DEFINES，<cmath> 才会暴露 M_PI
    target_compile_definitions(TreeGraphStudio PRIVATE _USE_MATH_DEFINES)
endif()

set_target_properties(TreeGraphStudio PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows: 自动复制 Qt 插件
if(WIN32)
    target_link_libraries(TreeGraphStudio PRIVATE dwmapi)
    set_target_properties(TreeGraphStudio PROPERTIES SUFFIX ".exe")
    
    # 查找并复制所有存在的插件
    add_custom_command(TARGET TreeGraphStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TreeGraphStudio>/mediaservice"
    )
    
    find_path(QT_PLUGIN_DIR NAMES mediaservice/dsengine.dll
        PATHS ${Qt5_DIR}/../../../plugins ${Qt5_DIR}/../..
    )
    
    if(QT_PLUGIN_DIR)
        file(GLOB MEDIA_DLLS "${QT_PLUGIN_DIR}/mediaservice/*.dll")
        foreach(dll ${MEDIA_DLLS})
            add_custom_command(TARGET TreeGraphStudio POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" 
                "$<TARGET_FILE_DIR:TreeGraphStudio>/mediaservice/"
            )
        endforeach()
    endif()
endif()
