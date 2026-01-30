cmake_minimum_required(VERSION 3.20)
project(nationwide)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(DEPS_DIR "${PROJECT_SOURCE_DIR}/deps")

file(GLOB_RECURSE SRC_FILES 
    "${SRC_DIR}/***.cpp"
    "${SRC_DIR}/***.hpp"
    "${DEPS_DIR}/**.c"
    "${DEPS_DIR}/**.cpp"
    "${DEPS_DIR}/**.h"
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} )
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)