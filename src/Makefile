# Stand alone Makefile for building most BILDE executables

# ubuntu 18.04: sudo apt install libopencv-dev libboost-dev libprotobuf-dev

# Authors: Logan Goldberg and Jerod Weinman

# Library dependencies
LIBS := opencv_ml opencv_objdetect opencv_stitching opencv_superres \
	opencv_videostab opencv_calib3d opencv_features2d opencv_highgui opencv_video \
	opencv_photo opencv_imgproc opencv_flann opencv_imgcodecs opencv_shape \
	opencv_core opencv_videoio \
	protobuf webp png tiff lzma jpeg z
#jasper  uuid 

# Build with g++
CXX := g++

# Flags
CXXFLAGS := -std=c++11
CPPFLAGS := -I../include -I/usr/include/opencv4
LDLIBS := $(addprefix -l, $(LIBS))

# Source files and target executables
SOURCES := $(wildcard *.cc)
TARGETS := $(basename ${SOURCES})

all: ${TARGETS} ../include/__bilde__.hpp

# If ../include/__bilde__.hpp has not been created create an empty one.
../include/__bilde__.hpp:
	touch ../include/__bilde__.hpp

# Uses Make's implicit rules to compile and link executables 

clean:
	rm -f ${TARGETS} ../include/__bilde__.hpp
