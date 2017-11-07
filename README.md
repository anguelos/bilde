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
