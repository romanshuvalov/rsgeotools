/*

Copyright (c) 2020, Roman Shuvalov, www.romanshuvalov.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <iostream>
#include <fstream>
#include <string>


#include <string.h>

#include <tiffio.h>

using namespace std;

char *filename;
char *out_filename;


typedef struct hm_file_header_t {
    uint32_t magic_num;
    uint32_t flags;
    uint32_t image_size;
    uint32_t reserved;

    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
} hm_file_header_t;

void DummyHandler(const char* module, const char* fmt, va_list ap)
{
    // ignore errors and warnings (or handle them your own way)
};

int main(int argc, char* argv[]) {

    TIFFSetWarningHandler(DummyHandler);

    if (argc != 5) {
        cerr << "Usage:  " << argv[0] << " <background.tiff> <overlay.tiff> <mask.tiff> <output_filename.tiff>" << endl;
        return -1;
    }

    char *fn_tiff1 = argv[1];
    char *fn_tiff2 = argv[2];
    char *fn_tiff_mask = argv[3];
    char *fn_tiff_out = argv[4];

    // only check
    FILE *fp;

    fp = fopen(fn_tiff1, "rb");
    if (!fp) {
        fprintf(stderr, "Not found: %s. Aborted.\n", fn_tiff1);
        return -1;
    };
    fclose(fp);

    fp = fopen(fn_tiff2, "rb");
    if (!fp) {
        fprintf(stderr, "Not found: %s. Aborted.\n", fn_tiff2);
        return -1;
    };
    fclose(fp);

    fp = fopen(fn_tiff_mask, "rb");
    if (!fp) {
        fprintf(stderr, "Not found: %s. Aborted.\n", fn_tiff_mask);
        return -1;
    };
    fclose(fp);


    TIFF *tiff1 = TIFFOpen(fn_tiff1, "rb");

    TIFF *tiff2 = TIFFOpen(fn_tiff2, "rb");

    TIFF *tiff_mask = TIFFOpen(fn_tiff_mask, "rb");

    TIFF *tiff_out = TIFFOpen(fn_tiff_out, "rb+");


    // #define uint32 unsigned long

    uint32_t width = 0, height = 0, bits_per_sample = 0, samples_per_pixel = 0, sample_format = 0;
//    uint32_t t[6];



    TIFFGetField(tiff1, TIFFTAG_IMAGEWIDTH, &width);           // uint32 width;
    TIFFGetField(tiff1, TIFFTAG_IMAGELENGTH, &height);        // uint32 height;

    TIFFGetField(tiff1, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetField(tiff1, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    TIFFGetField(tiff1, TIFFTAG_SAMPLEFORMAT, &sample_format);



    printf("%s: (%d x %d, %d bit) (samples per pixel: %d, format: %d) \n", fn_tiff1, width, height, bits_per_sample, samples_per_pixel, sample_format);
    //printf("sample format: %d, samples per pixel: %d\n", sample_format, samples_per_pixel);

    if ( (bits_per_sample != 16) || (sample_format != 2) || (samples_per_pixel != 1) ) {
        fprintf(stderr, "ERROR: Bad format. Aborted. \n\n");
        TIFFClose(tiff1);
        TIFFClose(tiff2);
        TIFFClose(tiff_mask);
        TIFFClose(tiff_out);
        exit(-1);
    };

    uint32_t width2 = 0, height2 = 0, bits_per_sample2 = 0, samples_per_pixel2 = 0, sample_format2 = 0;

    TIFFGetField(tiff2, TIFFTAG_IMAGEWIDTH, &width2);           // uint32 width;
    TIFFGetField(tiff2, TIFFTAG_IMAGELENGTH, &height2);        // uint32 height;

    TIFFGetField(tiff2, TIFFTAG_BITSPERSAMPLE, &bits_per_sample2);
    TIFFGetField(tiff2, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel2);
    TIFFGetField(tiff2, TIFFTAG_SAMPLEFORMAT, &sample_format2);

    printf("%s: (%d x %d, %d bit) (samples per pixel: %d, format: %d) \n", fn_tiff2, width2, height2, bits_per_sample2, samples_per_pixel2, sample_format2);

    if (
        (width != width2)
        || (height != height2)
        || (bits_per_sample != bits_per_sample2)
        || (samples_per_pixel != samples_per_pixel2)
        || (sample_format != sample_format2)

        ) {

        fprintf(stderr, "ERROR: Format of %s is different from %s. Aborted. \n", fn_tiff2, fn_tiff1 );

        TIFFClose(tiff1);
        TIFFClose(tiff2);
        TIFFClose(tiff_mask);
        TIFFClose(tiff_out);

        exit(-1);

    };



    uint32_t width_mask = 0, height_mask = 0, bits_per_sample_mask = 0, samples_per_pixel_mask = 0, sample_format_mask = 0;

    TIFFGetField(tiff_mask, TIFFTAG_IMAGEWIDTH, &width_mask);           // uint32 width;
    TIFFGetField(tiff_mask, TIFFTAG_IMAGELENGTH, &height_mask);        // uint32 height;

    TIFFGetField(tiff_mask, TIFFTAG_BITSPERSAMPLE, &bits_per_sample_mask);
    TIFFGetField(tiff_mask, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel_mask);
    TIFFGetField(tiff_mask, TIFFTAG_SAMPLEFORMAT, &sample_format_mask);

    printf("%s: (%d x %d, %d bit) (samples per pixel: %d, format: %d) \n", fn_tiff_mask, width_mask, height_mask, bits_per_sample_mask, samples_per_pixel_mask, sample_format_mask);


//    if (
//        (width != width2)
//        || (height != height2)
//        || (bits_per_sample != bits_per_sample2)
//        || (samples_per_pixel != samples_per_pixel2)
//        || (sample_format != sample_format2)
//
//        ) {
//
//        fprintf(stderr, "ERROR: Format of %s is different from %s. Aborted. \n", fn_tiff2, fn_tiff1 );
//
//        TIFFClose(tiff1);
//        TIFFClose(tiff2);
//        TIFFClose(tiff_mask);
//        TIFFClose(tiff_out);
//
//        exit(-1);
//
//    };





////    TIFFSetField(tiff_out, TIFFTAG_IMAGEWIDTH, width);
////    TIFFSetField(tiff_out, TIFFTAG_IMAGELENGTH, height);
////    TIFFSetField(tiff_out, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
////    TIFFSetField(tiff_out, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
////    TIFFSetField(tiff_out, TIFFTAG_SAMPLEFORMAT, sample_format);






    int16_t *buf1, *buf2;
    uint8_t *buf_mask;
    tsize_t scanline = TIFFScanlineSize(tiff1);

    if (TIFFScanlineSize(tiff2) != scanline) {
        fprintf(stderr, "ERROR: Scanline Size of %s (%d) is different from %s (%d).\n", fn_tiff2, (TIFFScanlineSize(tiff2)), fn_tiff1, scanline );
        return -1;
    };
    if (2*TIFFScanlineSize(tiff_mask) != scanline) {
        fprintf(stderr, "ERROR: Scanline Size of %s (%d) must be 1/2 of scanline size of %s (%d).\n", fn_tiff_mask, (TIFFScanlineSize(tiff_mask)), fn_tiff1, scanline );
        return -1;
    };


    buf1 = (int16_t*) _TIFFmalloc(scanline);
    buf2 = (int16_t*) _TIFFmalloc(scanline);
    buf_mask = (uint8_t*) _TIFFmalloc(scanline);


    for (uint32_t row = 0; row < height; row++) {

        TIFFReadScanline(tiff1, buf1, row);
        TIFFReadScanline(tiff2, buf2, row);
        TIFFReadScanline(tiff_mask, buf_mask, row);


        for (int i = 0; i < width; i++) {

            float k = 1.0 / 255.0 * buf_mask[i];

            buf1[i] = (int16_t) ( (1.0-k)*buf1[i] + k*buf2[i] );


//            buf1[i] = buf_mask[i];



        };


        TIFFWriteScanline(tiff_out, buf1, row);


        //printf("\n");
    };

    _TIFFfree(buf1);
    _TIFFfree(buf2);
    _TIFFfree(buf_mask);



    TIFFClose(tiff1);
    TIFFClose(tiff2);
    TIFFClose(tiff_mask);
    TIFFClose(tiff_out);


//    fclose(fp);

    printf("Done.\n");

    return 0;
}
