#*********************************************************************
# Copyright (C) Suirless, 2018-2020. All rights reserved.
# XPackage - package system for X-Project
# EULA License
#****************************************************************/

cmake_minimum_required(VERSION 3.16)
project(XPackage C CXX)
include(GitUtils.cmake)

set(TBB_BUILD_TESTS OFF)
set(TBB_INSTALL_TARGETS OFF)
set(BUILD_STAFF 1)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) 
set(XPACKAGE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XPACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SIMDJSON_JUST_LIBRARY ON)

if (MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

git_clone(
    PROJECT_NAME    simdjson
    GIT_URL         https://github.com/suirless/simdjson.git
    GIT_BRANCH      main
    DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/simdjson)

if (USE_VCPKG OR UNIX AND APPLE)
    find_package(ZLIB REQUIRED)
    find_package(PNG REQUIRED)
else()
    git_clone(
        PROJECT_NAME    zlib
        GIT_URL         https://github.com/suirless/zlib.git
        GIT_BRANCH      master
        DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}/deps
    )

    git_clone(
        PROJECT_NAME    libpng
        GIT_URL         https://github.com/suirless/libpng.git
        GIT_BRANCH      libpng16
        DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}/deps
    )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib)
    set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/libpng)
endif()

git_clone(
    PROJECT_NAME    zipper
    GIT_URL         https://github.com/suirless/zipper.git
    GIT_BRANCH      master
    DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/zipper)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/zipper/zipper)

file(GLOB XPACKAGE_BASE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

if (WIN32)
	file(GLOB XPACKAGE_BASE_PLATFORM_SRC
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/*.h
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/*.cpp
	)
elseif (UNIX AND NOT APPLE)
	file(GLOB XPACKAGE_BASE_PLATFORM_SRC
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/*.h
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/*.cpp
	)
else ()
	file(GLOB XPACKAGE_BASE_PLATFORM_SRC
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/*.h
	    ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/*.cpp
	)
endif()

file(GLOB XPACKAGE_SRC
    ${XPACKAGE_BASE_SRC}
    ${XPACKAGE_BASE_PLATFORM_SRC}
)

add_library(xpackage STATIC ${XPACKAGE_SRC})
target_include_directories(xpackage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/simdjson)
target_include_directories(xpackage PUBLIC ${XPACKAGE_INCLUDE_DIR})
target_link_libraries(xpackage PUBLIC png_static zlib simdjson staticZipper)