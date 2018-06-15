# Makefile for building BILDE executables

# Authors: Logan Goldberg and Jerod Weinman

# Library dependencies
LIBS := opencv_ml opencv_objdetect opencv_stitching opencv_superres opencv_videostab opencv_calib3d opencv_features2d opencv_highgui opencv_video opencv_photo opencv_imgproc opencv_flann opencv_core  protobuf webp png tiff jasper  uuid lzma jpeg z


# Cannot locate these libraries. Doesn't seem to interfere with the process.
# opencv_shape
# opencv_videoio
# opencv_imgcodecs
# comctl32
# comdlg32
# gdi32
# ole32
# setupapi
# ws2_32
# vfw32
# strmiids
# winmm
# glu32
# opengl32
# oleaut32
# IexMath Imath IlmImf Iex Half IlmThread (windows ImageMagick library ports?)

# Build with g++
CXX := g++

# Flags
CXXFLAGS := -std=c++11
CPPFLAGS := -I../include
LDFLAGS += $(addprefix -l, $(LIBS))

# Source files and target executables
SOURCES := $(wildcard *.cc)
TARGETS := $(basename ${SOURCES})

all: ${TARGETS}

# Uses Make's implicit rules to compile and link executables 

clean:
	rm -f {$TARGETS}
