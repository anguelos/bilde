cmake_minimum_required (VERSION 2.6)
project (Bilde)

set (Tutorial_VERSION_MAJOR 0)
set (Tutorial_VERSION_MINOR 1)


set (PY_V 3.4)
set (PYV 34)
set (ENABLE_PYTHON)

#set (OPENCV_MAJOR "3")
set (OPENCV_MAJOR "2")



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
include_directories ("${PROJECT_SOURCE_DIR}/include/containers")
include_directories ("${PROJECT_SOURCE_DIR}/include/io")
include_directories ("${PROJECT_SOURCE_DIR}/include/operations")
include_directories ("${PROJECT_SOURCE_DIR}/include/util")
include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${CMAKE_INCLUDE_PATH}")

# configure a header file to pass some of the CMake settings
# to the source code
add_library( libjpeg SHARED IMPORTED )
add_library( libz SHARED IMPORTED )
add_library( liblzma SHARED IMPORTED )
add_library( libtiff SHARED IMPORTED )

add_library( libopencv_highgui SHARED IMPORTED )
add_library( libopencv_core SHARED IMPORTED )


#*******************LIBRARIES******************************
#opopencv2_core
if(${OPENCV_MAJOR} STREQUAL "3" OR ${OPENCV_MAJOR} STREQUAL "2")
    find_library(
        OPENCV2_CORE_LIB opencv_core
        #"$PATHS" "${}"
        HINTS "/usr/local/lib" "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    if(${OPENCV2_CORE_LIB} STREQUAL "OPENCV2_CORE_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'OPENCV2 CORE' library")
    endif()
    message(STATUS "Found 'OPENCV2 CORE' at: ${OPENCV2_CORE_LIB}")

    if(${OPENCV_MAJOR} STREQUAL "3")
        find_library(
            OPENCV3_IMGCODECS_LIB opencv_imgcodecs
            HINTS "/usr/local/lib"  "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
        )

        if(${OPENCV3_IMGCODECS_LIB} STREQUAL "OPENCV3_IMGCODECS_LIB-NOTFOUND")
            message(FATAL_ERROR "Couldn't find the 'OPENCV3_IMGCODECS_LIB' library")
        endif()
        message(STATUS "Found 'OPENCV3 IMGCODECS' at: ${OPENCV3_IMGCODECS_LIB}")
    endif()

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


if(${ENABLE_PYTHON})

    find_library(
        PYTHON_LIB python${PY_V}m
        #"$PATHS" "${}"
        HINTS "/usr/local/lib" "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    if(${PYTHON_LIB} STREQUAL "PYTHON_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'PYTHON' ${PY_V} library")
    endif()
    message(STATUS "Found 'PYTHON ${PY_V}' at: ${PYTHON_LIB}")


    find_library(
        BOOST_PYTHON_LIB boost_python-py${PYV}
        #"$PATHS" "${}"
        HINTS  "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    if(${BOOST_PYTHON_LIB} STREQUAL "BOOST_PYTHON_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'BOOST PYTHON' boost_python-py${PYV} library")
    endif()
    message(STATUS "Found 'BOOST PYTHON ${PY_V}' at: ${BOOST_PYTHON_LIB}")

    #find_library(
    #    BOOST_SYSTEM_LIB boost_system-mt
    #    #"$PATHS" "${}"
    #    HINTS "/opt/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    #)
    #
    #if(${BOOST_SYSTEM_LIB} STREQUAL "BOOST_SYSTEM_LIB-NOTFOUND")
    #    message(FATAL_ERROR "Couldn't find the 'BOOST SYSTEM' library")
    #endif()
    message(STATUS "Found 'BOOST SYSTEM' at: ${BOOST_SYSTEM_LIB}")

    find_library(
        BOOST_NUMPY_LIB boost_numpy
        #"$PATHS" "${}"
        HINTS "/opt/local/lib" "/opt/local/local/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu"
    )
    if(${BOOST_NUMPY_LIB} STREQUAL "BOOST_NUMPY_LIB-NOTFOUND")
        message(FATAL_ERROR "Couldn't find the 'BOOST NUMPY' library")
    endif()
    message(STATUS "Found 'BOOST NUMPY' at: ${BOOST_NUMPY_LIB}")

endif()

configure_file (  "${PROJECT_SOURCE_DIR}/include/config.hpp.in"  "${PROJECT_BINARY_DIR}/include/config.hpp" )
include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE HEADERS *.hpp)


#*********************EXECUTABLES******************************
#enhace_grayscale

#add_executable(enhace_grayscale src/enhace_grayscale.cc)
#target_link_libraries (enhace_grayscale ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})

#add_executable(textline_segment src/textline_segment.cc)
#target_link_libraries (textline_segment ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})

if(${OPENCV_MAJOR} STREQUAL "3")#this is ugly
    add_executable(binarize src/binarize.cc ${HEADERS})
    target_link_libraries (binarize ${OPENCV2_CORE_LIB} ${OPENCV3_IMGCODECS_LIB} ${OPENCV2_HIGHGUI_LIB})

    add_executable(lbpFeatures src/lbpFeatures.cc ${HEADERS})
    target_link_libraries (lbpFeatures  ${OPENCV2_CORE_LIB} ${OPENCV3_IMGCODECS_LIB} ${OPENCV2_HIGHGUI_LIB})

    add_executable(lbpTransform src/lbpTransform.cc ${HEADERS})
    target_link_libraries (lbpTransform ${OPENCV2_CORE_LIB} ${OPENCV3_IMGCODECS_LIB} ${OPENCV2_HIGHGUI_LIB})


    add_executable(lbpFeatures2 ./src/lbpFeatures2.cc ${HEADERS})
    target_link_libraries (lbpFeatures2 ${OPENCV2_CORE_LIB} ${OPENCV3_IMGCODECS_LIB} ${OPENCV2_HIGHGUI_LIB})


    if(${ENABLE_PYTHON})
        add_library(npbilde SHARED src/npbilde.cc ${HEADERS})
        target_link_libraries (npbilde ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB} ${BOOST_PYTHON_LIB} ${BOOST_NUMPY_LIB} ${PYTHON_LIB})
    endif()
else()
    add_executable(binarize src/binarize.cc ${HEADERS})
    target_link_libraries (binarize ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})

    add_executable(lbpFeatures src/lbpFeatures.cc ${HEADERS})
    target_link_libraries (lbpFeatures  ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})

    add_executable(lbpTransform src/lbpTransform.cc ${HEADERS})
    target_link_libraries (lbpTransform ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})


    add_executable(lbpFeatures2 ./src/lbpFeatures2.cc ${HEADERS})
    target_link_libraries (lbpFeatures2 ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB})


    if(${ENABLE_PYTHON})
        add_library(npbilde SHARED src/npbilde.cc ${HEADERS})
        target_link_libraries (npbilde ${OPENCV2_CORE_LIB} ${OPENCV2_HIGHGUI_LIB} ${BOOST_PYTHON_LIB} ${BOOST_NUMPY_LIB} ${PYTHON_LIB})
    endif()
endif()
