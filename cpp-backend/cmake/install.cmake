file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

include_directories(include)

add_executable(${projectName}
    ${src_files}
    ${qrc_files}
)

# 子模块 HXLibs
add_subdirectory(lib/HXLibs)
target_link_libraries(${projectName} PUBLIC HXLibs)