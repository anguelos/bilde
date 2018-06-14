# Optimizied for Ubuntu 16.04
# Building lbpFeatures2.cc

# Libraries to dynamically link
LIBS := opencv_ml opencv_objdetect opencv_stitching opencv_superres opencv_videostab opencv_calib3d opencv_features2d opencv_highgui opencv_video opencv_photo opencv_imgproc opencv_flann opencv_core IexMath protobuf webp png tiff jasper Imath IlmImf Iex Half IlmThread uuid lzma jpeg z

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

# Build with g++
CC := g++

# Flags
CFLAGS := -std=c++11
LDFLAGS += $(addprefix -l, $(LIBS))

lbpFeatures2:
	$(CC) $(CFLAGS) src/lbpFeatures2.cc -o lbpFeatures2 -I./include $(LDFLAGS)

clean:
	rm ./lbpFeatures2
