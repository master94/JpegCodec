#include <iostream>
#include <vector>
#include <cmath>

#include "bitmap.h"
#include "jpegcodec.h"

void printData(const std::vector<int> &data)
{
    int c = 0;

    for (int x : data) {
        char buffer[4];
        memcpy(buffer, &x, sizeof(int));

        for (int i = sizeof(buffer) - 1; i >= 0; --i) {
            std::cout << int((unsigned char)buffer[i]) << " ";
        }
    }
    std::cout << std::endl;
}

int main()
{
    const int w = 8;
    const int h = 8;

    Bitmap b(w, h);
    for (int i = 0; i < b.height(); ++i) {
        for (int j = 0; j < b.width(); ++j) {
            /*int v = 0;
            if (i >= 8 && j < 8)
                v = 175;
            else if (i < 8 && j >= 8)
                v = 100;
            else if (i >= 8 && j >= 8)
                v = 255;*/

            if ((i + j) & 1)
            b.setPixel(j, i, Pixel(255, 255, 255));
        }
    }

    std::pair<std::vector<int>, int> data = JpegCodec::encode(b);
    JpegCodec::writeJpegToFile("/Users/user/Desktop/test.jpg", data, w ,h);
    printData(data.first);
    std::cout << data.second;

    return 0;
}

