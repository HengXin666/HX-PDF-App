file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

file(GLOB_RECURSE qrc_files CONFIGURE_DEPENDS 
    resources/*.qrc
)

include_directories(include)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${projectName}
    ${src_files}
    ${qrc_files}
)

target_link_libraries(${projectName}
    PRIVATE Qt::Core
    PRIVATE Qt::Gui
    PRIVATE Qt::Widgets
)

# Qt拓展 (PDF操作 与 渲染)
find_package(Qt6 REQUIRED COMPONENTS Pdf PdfWidgets)
target_link_libraries(${projectName} PRIVATE Qt6::Pdf Qt::PdfWidgets)

# Qt拓展 (音频播放)
# find_package(Qt6 REQUIRED COMPONENTS Multimedia)
# target_link_libraries(${projectName} PRIVATE Qt::Multimedia)

# Qt拓展 (SVG)
find_package(Qt6 REQUIRED COMPONENTS Svg)
target_link_libraries(${projectName} PRIVATE Qt::Svg)

# Qt拓展 (XML)
find_package(Qt6 REQUIRED COMPONENTS Xml)
target_link_libraries(${projectName} PRIVATE Qt::Xml)

# Qt拓展 (编码)
# find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
# target_link_libraries(${projectName} PRIVATE Qt::Core5Compat)

# Qt拓展 (并行库)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
target_link_libraries(${projectName} PRIVATE Qt::Concurrent)

# Qt拓展 (网络库)
find_package(Qt6 REQUIRED COMPONENTS Network)
target_link_libraries(${projectName} PRIVATE Qt::Network)

# 第三方依赖 (qpdf)
if (WIN32)
    # 不用这个库
    if(FALSE)
        list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/lib/qpdf")
        find_package(qpdf CONFIG REQUIRED)
        target_link_libraries(${projectName} PRIVATE qpdf::libqpdf)

        # 获取 qpdf 库的 DLL 文件路径
        get_target_property(QPDF_LIB_DIR qpdf::libqpdf LOCATION)
        get_filename_component(QPDF_DLL_PATH ${QPDF_LIB_DIR} DIRECTORY)
        set(QPDF_DLL ${QPDF_DLL_PATH}/qpdf30.dll)

        # 将 DLL 复制到可执行文件的目录
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${QPDF_DLL} $<TARGET_FILE_DIR:${projectName}>
        )
    endif()
    
    if(TRUE)
        include_directories("${PROJECT_SOURCE_DIR}/lib/mupdf/include")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Release/libmupdf.lib")
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Release/libthirdparty.lib")
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Release/libresources.lib")
        else()
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Debug/libmupdf.lib")
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Debug/libthirdparty.lib")
            target_link_libraries(${projectName} PRIVATE "${PROJECT_SOURCE_DIR}/lib/mupdf/lib/Debug/libresources.lib")
        endif()
    endif()
else()
    find_package(qpdf CONFIG REQUIRED)
    target_link_libraries(${projectName} PRIVATE qpdf::libqpdf)
endif()

if (WIN32)
    # 解决路径问题, 确保 windeployqt.exe 存在
    set(QT_BIN_DIR "${QT_COMPILER_PATH}/bin")
    if(NOT EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        message(FATAL_ERROR "Error: windeployqt.exe not found in ${QT_BIN_DIR}")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --debug "$<TARGET_FILE:${projectName}>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
        )
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release")
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --release "$<TARGET_FILE:${projectName}>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
        )
    endif()
endif()

set_target_properties(${projectName} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE OFF
    WIN32_EXECUTABLE OFF # 这里需要为 OFF 才可以让vscode在控制台中输出...
)

include(GNUInstallDirs)
install(TARGETS ${projectName}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
