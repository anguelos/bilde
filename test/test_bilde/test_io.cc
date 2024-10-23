#define CATCH_CONFIG_MAIN

#include <png.h>
#include <tiff.h>

#include "catch.hpp"  // -I/usr/include/catch2
#include "bilde.hpp"


bilde::Buffer<bilde::t_uint8> create_buffer(int width, int height, int offset=0){
    bilde::Buffer<bilde::t_uint8> res(width, height);
    bilde::t_sz x, y;
    bilde::t_uint8 * row;
    for(y=0;y<res.width;y++){
        row = res.getRow(y);
        for(x=0; x< res.width;x++){
            row[x]= (offset + x + y) % 256;
        }
    }
    return res;
}


TEST_CASE("Testing libpng IO") {
    /*REQUIRE(recurse(img, 101));*/

    SECTION("Testing grayscale saving and loading") {
        bilde::Buffer<bilde::t_uint8> img = create_buffer(1024,1024);
        bilde::io::png::savePngGrayscale("/tmp/hello.png", img);
        bilde::Buffer<bilde::t_uint8> img2 = bilde::io::png::loadPngGrayscale("/tmp/hello.png");
        REQUIRE(bilde::operations::essential::countEqual(img, img2)==img.width*img.height);
    }

    SECTION("Testing color saving and loading") {
        int width = 1024;
        int height = 1024;
        std::vector<bilde::Buffer<bilde::t_uint8>> img = {create_buffer(width,1024), create_buffer(width,1024, 64), create_buffer(width,1024, 128)};
        bilde::io::png::savePngColor("/tmp/hello_color.png", img);
        std::vector<bilde::Buffer<bilde::t_uint8>> img2 = bilde::io::png::loadPngColor("/tmp/hello_color.png");
        REQUIRE(bilde::operations::essential::countEqual(img[0], img2[0])==width*height);
        REQUIRE(bilde::operations::essential::countEqual(img[1], img2[1])==width*height);
        REQUIRE(bilde::operations::essential::countEqual(img[2], img2[2])==width*height);
    }
}


TEST_CASE("Testing libtiff IO") {
    /*REQUIRE(recurse(img, 101));*/

    SECTION("Testing grayscale saving and loading") {
        bilde::Buffer<bilde::t_uint8> img = create_buffer(1024,1024);
        bilde::io::png::savePngGrayscale("/tmp/hello.png", img);
        bilde::Buffer<bilde::t_uint8> img2 = bilde::io::png::loadPngGrayscale("/tmp/hello.png");
        REQUIRE(bilde::operations::essential::countEqual(img, img2)==img.width*img.height);
    }

    SECTION("Testing color saving and loading") {
        int width = 1024;
        int height = 1024;
        std::vector<bilde::Buffer<bilde::t_uint8>> img = {create_buffer(width,1024), create_buffer(width,1024, 64), create_buffer(width,1024, 128)};
        bilde::io::png::savePngColor("/tmp/hello_color.png", img);
        std::vector<bilde::Buffer<bilde::t_uint8>> img2 = bilde::io::png::loadPngColor("/tmp/hello_color.png");
        REQUIRE(bilde::operations::essential::countEqual(img[0], img2[0])==width*height);
        REQUIRE(bilde::operations::essential::countEqual(img[1], img2[1])==width*height);
        REQUIRE(bilde::operations::essential::countEqual(img[2], img2[2])==width*height);
    }
}