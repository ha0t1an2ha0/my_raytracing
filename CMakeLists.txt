cmake_minimum_required(VERSION 3.18)

project(my_raytracing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/external)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    # 如果找到了 OpenMP，添加 OpenMP 编译和链接标志
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    # 链接 OpenMP 库
endif()

add_executable(zrt 
    src/main.cc)

if(OpenMP_CXX_FOUND)
    target_link_libraries(zrt PUBLIC OpenMP::OpenMP_CXX)
endif()