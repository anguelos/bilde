/*
 * libpng.hpp
 *
 *  Created on: Apr 10, 2013
 *      Author: anguelos
 */

#ifndef LIBPNG_HPP_
#define LIBPNG_HPP_


namespace bilde{
namespace io{
namespace png{



#include <stdexcept>
#include <cstdio>
#include <vector>
#include <png.h>


Buffer<t_uint8> loadPngGrayscale(const char* filename) {
    FILE* infile = fopen(filename, "rb");
    if (!infile) {
        throw std::runtime_error("Cannot open PNG file");
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw std::runtime_error("Cannot create PNG read struct.");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        throw std::runtime_error("Cannot create PNG info struct.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(infile);
        throw std::runtime_error("Error reading PNG file.");
    }
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    
    Buffer<t_uint8> res(width, height);

    if (color_type != PNG_COLOR_TYPE_GRAY) {
        throw std::runtime_error("Only single-channel grayscale images are supported.");
    }

    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    png_bytep row_pointer = new png_byte[width];
    for (int y = 0; y < height; ++y) {
        png_read_row(png_ptr, row_pointer, NULL);
        t_uint8 * outRow = res.getRow(y);
        for (int x = 0; x < width; ++x) {
            outRow[x] = static_cast<t_uint8>(row_pointer[x]);
        }
    }

    delete[] row_pointer;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(infile);
    return res;
}

// Save PNG (grayscale) with compression level
void savePngGrayscale(const char* filename, const Buffer<t_uint8>& buffer, int compression_level = 6) {
    FILE* outfile = fopen(filename, "wb");
    if (!outfile) {
        throw std::runtime_error("Cannot open PNG file for writing.");
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw std::runtime_error("Cannot create PNG write struct.");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        throw std::runtime_error("Cannot create PNG info struct.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(outfile);
        throw std::runtime_error("Error writing PNG file.");
    }

    png_init_io(png_ptr, outfile);
    png_set_compression_level(png_ptr, compression_level);

    png_set_IHDR(png_ptr, info_ptr, buffer.width, buffer.height,
                    8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep row_pointer = new png_byte[buffer.width];
    for (int y = 0; y < buffer.height; y++) {
        const t_uint8 * row = buffer.getConstRow(y);
        for (int x = 0; x < buffer.width; x++) {
            row_pointer[x] = static_cast<png_byte>(row[x]);
        }
        png_write_row(png_ptr, row_pointer);
    }

    delete[] row_pointer;
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
}

    // Load PNG (color - RGB)
std::vector<Buffer<t_uint8>> loadPngColor(const char* filename) {
    FILE* infile = fopen(filename, "rb");
    if (!infile) {
        throw std::runtime_error("Cannot open PNG file");
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw std::runtime_error("Cannot create PNG read struct.");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        throw std::runtime_error("Cannot create PNG info struct.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(infile);
        throw std::runtime_error("Error reading PNG file.");
    }

    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);

    std::vector<Buffer<t_uint8>> res = {Buffer<t_uint8>(width, height), Buffer<t_uint8>(width, height), Buffer<t_uint8>(width, height)};
    t_uint8 * resRedRow;
    t_uint8 * resGreenRow;
    t_uint8 * resBlueRow;

    if (color_type != PNG_COLOR_TYPE_RGB) {
        throw std::runtime_error("Only 3-channel RGB images are supported.");
    }

    png_bytep row_pointer = new png_byte[width * 3];
    for (int y = 0; y < height; ++y) {
        png_read_row(png_ptr, row_pointer, NULL);
        resRedRow = res[0].getRow(y);
        resGreenRow = res[1].getRow(y);
        resBlueRow = res[2].getRow(y);
        for (int x = 0; x < width; ++x) {
            resRedRow[x] = static_cast<t_uint8>(row_pointer[x * 3]);
            resGreenRow[x] = static_cast<t_uint8>(row_pointer[x * 3 + 1]);
            resBlueRow[x] = static_cast<t_uint8>(row_pointer[x * 3 + 2]);
        }
    }

    delete[] row_pointer;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(infile);
    return res;
}

// Save PNG (color - RGB) with compression level
void savePngColor(const char* filename, const std::vector<Buffer<t_uint8>>& buffers, int compression_level = 6) {
    if (buffers.size() != 3) {
        throw std::runtime_error("Expected 3 buffers for RGB color image.");
    }

    FILE* outfile = fopen(filename, "wb");
    if (!outfile) {
        throw std::runtime_error("Cannot open PNG file for writing.");
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw std::runtime_error("Cannot create PNG write struct.");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        throw std::runtime_error("Cannot create PNG info struct.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(outfile);
        throw std::runtime_error("Error writing PNG file.");
    }

    png_init_io(png_ptr, outfile);
    png_set_compression_level(png_ptr, compression_level);

    png_set_IHDR(png_ptr, info_ptr, buffers[0].width, buffers[0].height,
                    8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep row_pointer = new png_byte[buffers[0].width * 3];
    for (int y = 0; y < buffers[0].height; y++) {
        const t_uint8* red_row = buffers[0].getConstRow(y);
        const t_uint8* green_row = buffers[1].getConstRow(y);
        const t_uint8* blue_row = buffers[2].getConstRow(y);
        for (int x = 0; x < buffers[0].width; x++) {
            row_pointer[x * 3] = static_cast<png_byte>(red_row[x]);
            row_pointer[x * 3 + 1] = static_cast<png_byte>(green_row[x]);
            row_pointer[x * 3 + 2] = static_cast<png_byte>(blue_row[x]);
        }
        png_write_row(png_ptr, row_pointer);
    }

    delete[] row_pointer;
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
}

}
}
}



#endif /* LIBPNG_HPP_ */
