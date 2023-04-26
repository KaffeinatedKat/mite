cmake_minimum_required(VERSION 3.15)

project(mite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC ${SRC_ROOT}/*.cpp)
file(GLOB_RECURSE HEADERS ${HEADER_ROOT}/*.hpp)

set(INCLUDE_DIRS ${HEADER_ROOT})

add_executable(mite ${SRC} ${HEADERS})

target_include_directories(mite PRIVATE ${INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE STREQUAL "debug")
    target_compile_options(mite PRIVATE -Og -ggdb3 -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "release")
    target_compile_options(mite PRIVATE -O2 -flto=thin)
elseif(CMAKE_BUILD_TYPE STREQUAL "windows")
    target_compile_options(mite PRIVATE -DWINDOWS)
endif()


target_link_libraries(mite PRIVATE ${LIB})

add_custom_target(format COMMAND clang-format -i ${SRC} ${HEADERS})


