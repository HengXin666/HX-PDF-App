cmake_minimum_required(VERSION 3.16)

set(projectName "HX-PDF-Server")

project(${projectName}
    VERSION 0.0.1
    DESCRIPTION ${projectName}
    HOMEPAGE_URL "https://github.com/HengXin666/HX-PDF-App"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE "Debug")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("=-=-=-=-=-=-=当前是 [Release] 模式=-=-=-=-=-=-=")
else()
    message("=-=-=-=-=-=-=当前是 [DEBUG] 模式=-=-=-=-=-=-=")
    add_definitions(-D_HX_DEBUG_) # 添加一个 _HX_DEBUG_ 宏
endif()

# 加载依赖
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Find/)

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HX-PDF-Server")
    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
else()
    # todo
endif()
