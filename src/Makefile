# Makefile for building most BILDE executables

# Authors: Logan Goldberg and Jerod Weinman

# Library dependencies
LIBS := opencv_ml opencv_objdetect opencv_stitching opencv_superres \
	opencv_videostab opencv_calib3d opencv_features2d opencv_highgui opencv_video \
	opencv_photo opencv_imgproc opencv_flann opencv_imgcodecs opencv_shape \
	opencv_core opencv_videoio \
	protobuf webp png tiff uuid lzma jpeg z

# Build with g++
CXX := g++

# Flags
CXXFLAGS := -std=c++11
CPPFLAGS := -I../include -I/usr/include/opencv4/
LDLIBS := $(addprefix -l, $(LIBS))

# Source files and target executables
SOURCES := $(wildcard *.cc)
TARGETS := $(basename ${SOURCES})

all: ${TARGETS}

# Uses Make's implicit rules to compile and link executables 

clean:
	rm -f ${TARGETS}
