# MIT License
#
# Copyright (c) 2018-2025 Jakub Melka and Contributors
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.16)

set(PDF4QT_VERSION 1.6.0.0)
message(STATUS "Build type - CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

project(pdf_simplify VERSION ${PDF4QT_VERSION} LANGUAGES CXX)

# ─── Options ────────────────────────────────────────────────────────────────
option(PDF4QT_BUILD_TESTS "Build tests" OFF)

add_compile_definitions(PDF4QT_PROJECT_VERSION="${PDF4QT_VERSION}")
add_compile_definitions(QT_NO_EMIT)

# ─── C++ Standard ───────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Qt6 ────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Xml)

if(PDF4QT_BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

qt_standard_project_setup()

# ─── Third-party dependencies ───────────────────────────────────────────────
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenJPEG CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(blend2d CONFIG REQUIRED)

if(VCPKG_TOOLCHAIN)
    find_package(lcms2 REQUIRED)
    set(LCMS2_LIBRARIES lcms2::lcms2 CACHE INTERNAL "LCMS2 libraries")
else()
    find_library(LCMS2_LIBRARIES lcms2 REQUIRED)
endif()

# ─── Qt auto-generators ─────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTORCC_OPTIONS "--threshold;0;--compress;9")

# ─── Compiler flags ─────────────────────────────────────────────────────────
if (MSVC)
    add_compile_options(/bigobj /MP /W4 /wd5054 /wd4127 /wd4702)
endif()

# ─── Subdirectories ─────────────────────────────────────────────────────────
add_subdirectory(library)
add_subdirectory(app)
