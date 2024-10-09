# bilde

## (B)uffer (I)maging (L)ibrary, (D)irty and (E)xtendable

A library for writing procedural computer vision code in C++

* Header only
No linking complications!
Just copy it in your source tree.

* High C++ template use

* Define algorithms once, use them directly on several back-ends.
With a mechanism based on implicit template instantiation, an algorithm is defined as a single function, and will seemlesly run several image containers such as cv::Mat, IplImage, octave arrays, numpy arrays.


## Demos:

* SRS-LBP feature extractor:

Cross compiling for win32 (static build) and running in debian based linux.

Install dependencies:
```bash
sudo apt-get install autoconf automake autopoint bash bison bzip2 flex gettext git g++ gperf intltool libffi-dev libgdk-pixbuf2.0-dev libtool-bin libltdl-dev libssl-dev libxml-parser-perl make openssl p7zip-full patch perl pkg-config python ruby scons sed unzip wget xz-utils g++-multilib libc6-dev-i386
export MXE_ROOT="$HOME/tools/mxe"
mkdir -p "$HOME/tools"
cd "$HOME/tools"
git clone https://github.com/mxe/mxe.git
cd mxe
make opencv boost
```
Compile:
```bash
./compile_static_mxe.sh ./src/lbpFeatures2.cc /tmp/lbp_features2.exe
```

Get help:
```bash
wine /tmp/lbp_features2.exe
```

Run:
```bash
wine /tmp/lbp_features2.exe -i ./sample_data/PICT2466.png > /tmp/features.csv
```

Compiling in ubuntu:
```bash
sudo apt-get install libopencv-dev libhighgui-dev libprotobuf-dev libwebp-dev #  depending on your system you might need other packages
cd src
make lbpFeatures2
```

## Python extention (pybilde)
* Install build dependencies
```bash
pip install cibuildwheel
```

Build bdist without a CI
```bash
CIBW_BUILD="cp313t-manylinux_x86_64 cp313t-manylinux_x86_64 cp312-manylinux_x86_64 cp311-manylinux_x86_64 cp310-manylinux_x86_64 cp39-manylinux_x86_64 cp38-manylinux_x86_64 cp37-manylinux_x86_64 cp36-manylinux_x86_64" CIBW_BEFORE_BUILD="yum install -y boost-devel" cibuildwheel --platform linux --output-dir wheelhouse
CIBW_SKIP="pp310-manylinux_i686 pp39-manylinux_i686 pp38-manylinux_i686 pp37-manylinux_i686 pp36-manylinux_i686 pp310-manylinux_x86_64  pp39-manylinux_x86_64 pp38-manylinux_x86_64  pp37-manylinux_x86_64 pp36-manylinux_x86_64" CIBW_BEFORE_BUILD="yum install -y boost-devel" cibuildwheel --platform linux --output-dir wheelhouse
```
