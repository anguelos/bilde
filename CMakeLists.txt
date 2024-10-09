cmake_minimum_required (VERSION 3.12)
project (Bilde)

set (Tutorial_VERSION_MAJOR 0)
set (Tutorial_VERSION_MINOR 1)

option(ENABLE_CVMAT "Opencv>=2 containers, required for bilde programs" ON)
option(ENABLE_PYBIND11 "Python bindings to bilde" ON)
option(ENABLE_TIFFIO "Enable feature X in the project" OFF)


#set (OPENCV_MAJOR "3")
#set (OPENCV_MAJOR "4")



#set (PY_V 2.7)
#set (PYV 27)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#if(${APPLE})
    set(DIST_ROOT /opt/local)
#assume macports are installed on /opt/local
#and that gcc 4.8 and opencv ports are installed
    #set (CMAKE_CXX_COMPILER  /opt/local/bin/g++-mp-4.8)
    #set (CMAKE_CXX_COMPILER  /usr/bin/g++)
    #set (CMAKE_MAKE_PROGRAM /opt/local/bin/gmake)
    set (CMAKE_MAKE_PROGRAM /usr/bin/g++)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib /opt/local/lib /opt/local/local/lib)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /opt/local/include /usr/local/include  /opt/local/local/include  /opt/local/Library/Frameworks/Python.framework/Versions/${PY_V}/include/python${PY_V}m )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 ")
#elseif(${MXESTATIC})
#    set(DIST_ROOT "/Users/anguelos/mxe2")
#    set (CMAKE_CXX_COMPILER  "${MXE_ROOT}/usr/bin/i686-pc-mingw32.static-g++")
#    #fix gmake
#    set (CMAKE_MAKE_PROGRAM /opt/local/bin/gmake)
#    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${DIST_ROOT}/usr/i686-pc-mingw32.static/x86/mingw/staticlib/")
#    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${DIST_ROOT}usr/i686-pc-mingw32.static/include/")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
################################################
#elseif(${DEBIAN})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}/usr/include/lib /usr/include /usr/include/x86_64-linux-gnu/python${PY_V}m/ /usr/include/python${PY_V}m/ /usr/include/x86_64-linux-gnu/python${PY_V}/ /usr/include/python${PY_V}/)
endif()
#******************MACPORTS COMPATIBILITY*****************

include_directories ("${PROJECT_SOURCE_DIR}/include")
#include_directories ("${PROJECT_SOURCE_DIR}/include/containers")
#include_directories ("${PROJECT_SOURCE_DIR}/include/io")
#include_directories ("${PROJECT_SOURCE_DIR}/include/operations")
#include_directories ("${PROJECT_SOURCE_DIR}/include/util")
#include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${CMAKE_INCLUDE_PATH}")

# configure a header file to pass some of the CMake settings
# to the source code
add_library( libjpeg SHARED IMPORTED )
add_library( libz SHARED IMPORTED )
add_library( liblzma SHARED IMPORTED )
add_library( libtiff SHARED IMPORTED )

add_library( libopencv_highgui SHARED IMPORTED )
add_library( libopencv_core SHARED IMPORTED )


find_package(Boost REQUIRED COMPONENTS system filesystem)

if(ENABLE_CVMAT)
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
if(OpenCV_VERSION_MAJOR GREATER 2)
    find_library(
        OPENCV2_CORE_LIB opencv_core
        #"$PATHS" "${}"
        HINTS "/usr/local/lib" "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    if(${OPENCV2_CORE_LIB} STREQUAL "OPENCV2_CORE_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'OPENCV2 CORE' library")
    endif()
    message(STATUS "Found 'OPENCV2 CORE' at: ${OPENCV2_CORE_LIB}")

    find_library(
        OPENCV3_IMGCODECS_LIB opencv_imgcodecs
        HINTS "/usr/local/lib"  "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )

    if(${OPENCV3_IMGCODECS_LIB} STREQUAL "OPENCV3_IMGCODECS_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'OPENCV3_IMGCODECS_LIB' library")
    endif()
    message(STATUS "Found 'OPENCV3 IMGCODECS' at: ${OPENCV3_IMGCODECS_LIB}")

    find_library(
        OPENCV2_HIGHGUI_LIB opencv_highgui
        HINTS "/usr/local/lib"  "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    #opencv2_highgui
    if(${OPENCV2_HIGHGUI_LIB} STREQUAL "OPENCV2_HIGHGUI_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'OPENCV2 HIGHGUI' library")
    endif()
    message(STATUS "Found 'OPENCV2 HIGHGUI' at: ${OPENCV2_HIGHGUI_LIB}")
endif()

configure_file ("${PROJECT_SOURCE_DIR}/include/config.hpp.in"  "${PROJECT_BINARY_DIR}/include/config.hpp")
include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE HEADERS *.hpp)


#*********************EXECUTABLES******************************
add_executable(bilde_binarize src/binarize.cc ${HEADERS})
target_link_libraries (bilde_binarize PRIVATE ${OpenCV_LIBS})

add_executable(bilde_lbpFeatures2 src/lbpFeatures2.cc ${HEADERS})
target_link_libraries (bilde_lbpFeatures2 PRIVATE ${OpenCV_LIBS})

add_executable(bilde_enhace_grayscale src/enhace_grayscale.cc ${HEADERS})
target_link_libraries (bilde_enhace_grayscale PRIVATE ${OpenCV_LIBS})

add_executable(bilde_textline_segment src/textline_segment.cc ${HEADERS})
target_link_libraries (bilde_textline_segment PRIVATE ${OpenCV_LIBS})

# Needs debugging, doesn't compile for now
#add_executable(bilde_wi_retrieve src/wi_retrieve.cc ${HEADERS})
#target_link_libraries (bilde_wi_retrieve PRIVATE ${OpenCV_LIBS})

endif()  # ENABLE_CVMAT

if(ENABLE_PYBIND11)
#    file(GLOB PYTHON_INCLUDE_DIR "${PYTHON_VENV_PATH}/include/python3.*")
#    find_package(pybind11 REQUIRED
#    HINTS "/usr/local/lib" "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu" "${VIRTUAL_ENVz}"
#    )
#    pybind11_add_module(bilde src/bildepybind.cc ${HEADERS})
#    target_link_libraries (bilde PRIVATE ${OpenCV_LIBS} Boost::math Boost::shared_ptr)
endif()