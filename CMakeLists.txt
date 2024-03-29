#*********************************************************************
# Copyright (C) Suirless, 2018-2021. All rights reserved.
# XPackage - package system for X-Project
# Apache-2 License
#****************************************************************/

cmake_minimum_required(VERSION 3.16)
project(XPackage C CXX)

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

option(XPACKAGE_ENABLE_TESTS "Enable tests for XPackage" OFF)

if (MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/simdjson)

if (USE_VCPKG OR UNIX)
    find_package(ZLIB REQUIRED)
    find_package(PNG REQUIRED)
    set(BUILD_WITH_PACKAGES ON)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libpng)
    set(BUILD_WITH_PACKAGES OFF)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zipper)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zipper/zipper)

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
target_include_directories(xpackage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/simdjson)
target_include_directories(xpackage PUBLIC ${XPACKAGE_INCLUDE_DIR})

if (BUILD_WITH_PACKAGES)
	target_link_libraries(xpackage PUBLIC PNG::PNG ZLIB::ZLIB simdjson staticZipper)
else()
	target_link_libraries(xpackage PUBLIC png_static zlib simdjson staticZipper)
endif()

if (XPACKAGE_ENABLE_TESTS)
    add_executable(xpackage-test ${CMAKE_CURRENT_SOURCE_DIR}/src/test/main.cpp)
    target_link_libraries(xpackage-test xpackage)
endif()
