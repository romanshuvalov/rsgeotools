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



void DummyHandler(const char* module, const char* fmt, va_list ap)
{
    // ignore errors and warnings (or handle them your own way)
};

int main(int argc, char* argv[]) {

    TIFFSetWarningHandler(DummyHandler);

    if (argc != 3) {
        cerr << "Usage:  " << argv[0] << " FILE.TIFF OUTPUT_FILENAME" << endl;
        return -1;
    }

    filename = argv[1];
    out_filename = argv[2];


    printf("Converting %s to %s: ", filename, out_filename);

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        cout << "FP is null" << endl;
        return -1;
    };
    fclose(fp);


    TIFF *tif=TIFFOpen(filename, "r");
    fp = fopen(out_filename, "wb");


    uint32_t width = 0, height = 0, bits_per_sample = 0, samples_per_pixel = 0, sample_format = 0;


    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);           // uint32 width;
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);        // uint32 height;

    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);


    printf("(%d x %d, %d bit) ", width, height, bits_per_sample);

    if ( (bits_per_sample != 16) || (sample_format != 2) || (samples_per_pixel != 1) ) {
        printf("ERROR: Bad format. Aborted. \n\n");
        fclose(fp);
        TIFFClose(tif);
        return -1;
    }


    int16_t *buf;
    tsize_t scanline = TIFFScanlineSize(tif);


    buf = (int16_t*) _TIFFmalloc(scanline);
    for (uint32_t row = 0; row < height; row++) {
        TIFFReadScanline(tif, buf, row);


        for (int i = 0; i < width; i++) {
            buf[i] /= 2;
        }

        fwrite( buf, scanline, 1, fp );

    };
    _TIFFfree(buf);



    TIFFClose(tif);
    fclose(fp);

    printf("Done.\n");

    return 0;
}
