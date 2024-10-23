#define CATCH_CONFIG_MAIN

#include "catch.hpp"  // -I/usr/include/catch2
#include "bilde.hpp"


int recurse(bilde::Buffer<bilde::t_uint8> img, int depth=100) {
    if (depth>0){
        bilde::operations::essential::__setTo__<bilde::t_uint8>(img, depth);
        int res = recurse(img, depth-1);
        int max_found = bilde::operations::essential::getMaxValue(img);
        return (max_found > 0) * res;
    }
    return 1;
}


bilde::Buffer<bilde::t_uint8> create_buffer(int width, int height, bilde::t_uint8 val){
    bilde::Buffer<bilde::t_uint8> res(width, height);
    bilde::operations::essential::__setTo__<bilde::t_uint8>(res, val);
    return res;
}


TEST_CASE("Testing my template class") {
    bilde::Buffer<bilde::t_uint8> img(100, 100);
    //testing the copy constructor
    REQUIRE(recurse(img, 101));

    SECTION("Testing copy constructor and the smart pointer") {
        int width = 10;
        int height = 11;
        int fill = 7;
        bilde::Buffer<bilde::t_uint8> returned_img = create_buffer(width, height, fill);
        REQUIRE(bilde::operations::essential::getSum(returned_img) == width*height*fill);
    }

    SECTION("Testing for memory leaks") {
        int width = 10000;
        int height = 10000;
        int repetitions = 1000;  // 100 GB
        for(int n =0; n < repetitions; n++){
            bilde::Buffer<bilde::t_uint8> img = bilde::Buffer<bilde::t_uint8>(width, height);
        }        
    }
}