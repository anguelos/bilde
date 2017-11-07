#!/bin/bash

# Usage:
# compile_static_mxe.sh PATH_TO_MAIN_CC_FILE PATH_TO_WIN32_EXE
#
# instaling mxe:
#
# sudo apt-get install autoconf automake autopoint bash bison bzip2 flex gettext git g++ gperf intltool libffi-dev libgdk-pixbuf2.0-dev libtool-bin libltdl-dev libssl-dev libxml-parser-perl make openssl p7zip-full patch perl pkg-config python ruby scons sed unzip wget xz-utils g++-multilib libc6-dev-i386
# export MXE_ROOT="$HOME/tools/mxe"
# mkdir -p "$HOME/tools"
# cd "$HOME/tools"
# git clone https://github.com/mxe/mxe.git
# cd mxe
# make opencv boost

MXE_ROOT="$HOME/tools/mxe"

# list of dependencies obtained with: ~/tools/mxe/usr/bin/i686-w64-mingw32.static-pkg-config opencv --libs

"$MXE_ROOT/usr/bin/i686-w64-mingw32.static-g++" -std=c++11 $1 -o $2 -I./include "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_dnn.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_ml.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_objdetect.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_shape.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_stitching.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_superres.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_videostab.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_calib3d.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_features2d.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_highgui.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_videoio.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_imgcodecs.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_video.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_photo.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_imgproc.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_flann.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopencv_core.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libIexMath.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libprotobuf.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libcomctl32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libcomdlg32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libgdi32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libole32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libsetupapi.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libws2_32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libvfw32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libstrmiids.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libwebp.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libpng.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libtiff.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libjasper.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libImath.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libIlmImf.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libIex.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libHalf.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libIlmThread.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libwinmm.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libglu32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libopengl32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libuuid.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/liboleaut32.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/liblzma.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libjpeg.a" "${MXE_ROOT}/usr/i686-w64-mingw32.static/lib/libz.a"

