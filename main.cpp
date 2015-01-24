#include <iostream>
#include <vector>

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
    Bitmap b(8, 8);
    for (int i = 0; i < b.height(); ++i) {
        for (int j = 0; j < b.width(); ++j) {
            if (~(i + j) & 1) {
                b.setPixel(j, i, Pixel(255, 255, 255));
            }
        }
    }

    std::pair<std::vector<int>, int> data = JpegCodec::encode(b);
    JpegCodec::writeJpegToFile("/Users/user/Desktop/test.jpg", data);
    printData(data.first);
    std::cout << data.second;

    return 0;
}

