#include "jpegcodec.h"

#include <queue>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "bitmap.h"
#include "bitdatabuilder.h"

namespace
{

const char dcHuffmanNodes[17] = { 0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };
const char dcHuffmanValues[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
const int dcHuffmanLength[12] = { 2,3,3,3,3,3,4,5,6,7,8,9 };
const int dcHuffmanTable[12] = {
    0x000,0x002,0x003,0x004,0x005,0x006,
    0x00e,0x01e,0x03e,0x07e,0x0fe,0x1fe
};

const char acHuffmanNodes[17] = { 0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d };
const unsigned char acHuffmanValues[162] = {
    0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12, /* 0x00: EOB */
    0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
    0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,
    0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0, /* 0xf0: ZRL */
    0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,
    0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,
    0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
    0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
    0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
    0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
    0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
    0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
    0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
    0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,
    0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,
    0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,
    0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,
    0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,
    0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,
    0xf9,0xfa
};

const int acHuffmanLength[256] = {
     4, 2, 2, 3, 4, 5, 7, 8,
    10,16,16, 0, 0, 0, 0, 0,
     0, 4, 5, 7, 9,11,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 5, 8,10,12,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 6, 9,12,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 6,10,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 7,11,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 7,12,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 8,12,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 9,15,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 9,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0, 9,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0,10,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0,10,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0,11,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
     0,16,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0,
    11,16,16,16,16,16,16,16,
    16,16,16, 0, 0, 0, 0, 0
};

const int acHuffmanTable[256] = {
    0x000a,0x0000,0x0001,0x0004,0x000b,0x001a,0x0078,0x00f8,
    0x03f6,0xff82,0xff83,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x000c,0x001b,0x0079,0x01f6,0x07f6,0xff84,0xff85,
    0xff86,0xff87,0xff88,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x001c,0x00f9,0x03f7,0x0ff4,0xff89,0xff8a,0xff8b,
    0xff8c,0xff8d,0xff8e,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x003a,0x01f7,0x0ff5,0xff8f,0xff90,0xff91,0xff92,
    0xff93,0xff94,0xff95,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x003b,0x03f8,0xff96,0xff97,0xff98,0xff99,0xff9a,
    0xff9b,0xff9c,0xff9d,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x007a,0x07f7,0xff9e,0xff9f,0xffa0,0xffa1,0xffa2,
    0xffa3,0xffa4,0xffa5,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x007b,0x0ff6,0xffa6,0xffa7,0xffa8,0xffa9,0xffaa,
    0xffab,0xffac,0xffad,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x00fa,0x0ff7,0xffae,0xffaf,0xffb0,0xffb1,0xffb2,
    0xffb3,0xffb4,0xffb5,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x01f8,0x7fc0,0xffb6,0xffb7,0xffb8,0xffb9,0xffba,
    0xffbb,0xffbc,0xffbd,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x01f9,0xffbe,0xffbf,0xffc0,0xffc1,0xffc2,0xffc3,
    0xffc4,0xffc5,0xffc6,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x01fa,0xffc7,0xffc8,0xffc9,0xffca,0xffcb,0xffcc,
    0xffcd,0xffce,0xffcf,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x03f9,0xffd0,0xffd1,0xffd2,0xffd3,0xffd4,0xffd5,
    0xffd6,0xffd7,0xffd8,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x03fa,0xffd9,0xffda,0xffdb,0xffdc,0xffdd,0xffde,
    0xffdf,0xffe0,0xffe1,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x07f8,0xffe2,0xffe3,0xffe4,0xffe5,0xffe6,0xffe7,
    0xffe8,0xffe9,0xffea,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0xffeb,0xffec,0xffed,0xffee,0xffef,0xfff0,0xfff1,
    0xfff2,0xfff3,0xfff4,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x07f9,0xfff5,0xfff6,0xfff7,0xfff8,0xfff9,0xfffa,0xfffb,
    0xfffc,0xfffd,0xfffe,0x0000,0x0000,0x0000,0x0000,0x0000
};

    const int quantTable[64] = {
        16, 11, 10, 16, 24, 40, 51, 61,
        12, 12, 14, 19, 26, 58, 60, 55,
        14, 13, 16, 24, 40, 57, 69, 56,
        14, 17, 22, 29, 51, 87, 80, 62,
        18, 22, 37, 56, 68, 109, 103, 77,
        24, 35, 55, 64, 81, 104, 113, 92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103, 99
    };

    const int zigZagOrder[64] = {
         0,  1,  8, 16,  9,  2,  3, 10,
        17, 24, 32, 25, 18, 11,  4,  5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13,  6,  7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63
    };

    int codeLength(int x) {
        x = abs(x);

        for (int i = 8 * sizeof(int) - 1; i >= 0; --i) {
            if (x & (1 << i))
                return i + 1;
        }

        return 0;
    }

    void moveZigZag(int &x, int &y, int n)
    {
        if (y < n - 1) {
            x = fmax(0, x - 1);
            y = y + 1;
        } else {
            x = x + 1;
        }
    }
}


JpegCodec::JpegCodec()
{
}

std::pair<std::vector<int>, int> JpegCodec::encode(const Bitmap &bmp)
{
    if (!bmp.isValid())
        return std::pair<std::vector<int>, int>();

    const int size = bmp.width() * bmp.height();

    char y[size];
    char cb[size];
    char cr[size];

    const Pixel *data = bmp.data();
    for (int i = 0; i < size; ++i) {
        const Pixel &p = data[i];
        y[i] = 0.299 * p.red() + 0.587 * p.green() + 0.114 * p.blue() - 128;
        cb[i] = -0.1687 * p.red() - 0.3313 * p.green() + 0.5 * p.blue() + 128 - 128;
        cr[i] = 0.5 * p.red() - 0.4187 * p.green() - 0.0813 * p.blue() + 128 - 128;
    }

    BitDataBuilder builder;
    dct(y, bmp.width(), bmp.height(), builder);
    return builder.data();
}

void JpegCodec::dct(char *data, int width, int height, BitDataBuilder &builder)
{
    const int blockSize = 8;
    const int bufferSize = blockSize * blockSize;

    char byteBuffer[bufferSize];
    int intBuffer1[bufferSize];
    int intBuffer2[bufferSize];

    int prevDc = 0;

    for (int i = 0; i < ceil(float(height) / blockSize); ++i) {
        for (int j = 0; j < ceil(float(width) / blockSize); ++j) {
            memset(byteBuffer, 0, bufferSize);
            memset(intBuffer1, 0, bufferSize);
            memset(intBuffer2, 0, bufferSize);

            for (int k = 0; k < blockSize; ++k) {
                if (i * blockSize + k >= height)
                    break;

                const int pos = (i * blockSize + k) * width + j * blockSize;
                const int copySize = fmin(blockSize, width - j * blockSize);
                memcpy(byteBuffer + blockSize * k, data + pos, copySize);

                if (copySize < blockSize)
                    memset(byteBuffer + blockSize * k + copySize, byteBuffer[blockSize * k + copySize], blockSize - copySize);
            }

            dctBlock(byteBuffer, intBuffer1, blockSize);
            quantizeBlock(intBuffer1, blockSize);

            for (int a = 0; a < blockSize; ++a) {
                for (int b = 0; b < blockSize; ++b) {
                    std::cout << intBuffer1[a * blockSize + b] << " ";
                }
                std::cout << std::endl;
            }

            const int len = runLengthEncoding(intBuffer1, intBuffer2, blockSize);

            for (int a = 0; a < len; ++a) {
                std::cout << intBuffer2[a] << " ";
            }
            std::cout << std::endl;

            int data =  intBuffer2[0] - prevDc;
            prevDc = intBuffer2[0];
            const int dcCodeLength = codeLength(data);
            if (data < 0)
                data = ~(-data);

            std::cout << "DC " << dcHuffmanTable[dcCodeLength] << " " << dcHuffmanLength[dcCodeLength] << std::endl;
            std::cout << data << " " << dcCodeLength << std::endl;

            builder.appendData(dcHuffmanTable[dcCodeLength], dcHuffmanLength[dcCodeLength]);
            builder.appendData(data, dcCodeLength);

            variableLenghtHuffmanCoding(intBuffer2, len, builder);
        }
    }
}

void JpegCodec::dctBlock(const char *input, int *output, int blockSize)
{
    for (int v = 0; v < blockSize; ++v) {
        for (int u = 0; u < blockSize; ++u) {
            output[v * blockSize + u] = dctPixel(input, blockSize, u, v);
        }
    }
}

void JpegCodec::quantizeBlock(int *data, int blockSize)
{
    if (blockSize != 8)
        return;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            data[i * blockSize + j] = int(data[i * blockSize + j] / quantTable[i * blockSize + j]);
        }
    }
}

double JpegCodec::dctPixel(const char *data, int blockSize, int u, int v)
{
    double val = 0;

    for (int x = 0; x < blockSize; ++x) {
        for (int y = 0; y < blockSize; ++y) {
            val += data[y * blockSize + x] * cos((2 * x + 1) * u * M_PI / 16.0) * cos((2 * y + 1) * v * M_PI / 16.0);
        }
    }

    val *= (u == 0 && v == 0) ? 0.125 : 0.25;
    return val;
}

int JpegCodec::runLengthEncoding(int *input, int *output, int blockSize)
{
    int x = 0;
    int y = 0;

    int total = 0;
    int zeros = 0;
    const int maxZeros = 15;

    std::queue<int> values;

    while (x < blockSize && y < blockSize) {
        if (x == 0 && y == 0) {
            output[total++] = input[y * blockSize + x];
        } else {
            if (input[y * blockSize + x] == 0) {
                ++zeros;

                if (zeros == maxZeros) {
                    values.push(maxZeros);
                    values.push(0);
                    zeros = 0;
                }
            } else {
                while (!values.empty()) {
                    output[total++] = values.front();
                    values.pop();
                }

                output[total++] = zeros;
                output[total++] = input[y * blockSize + x];
                zeros = 0;
            }
        }

        if ((x + y) & 1)
            moveZigZag(x, y, blockSize);
        else
            moveZigZag(y, x, blockSize);
    }

    if (input[63] == 0) {
        output[total++] = 0;
        output[total++] = 0;
    }

    return total;
}

void JpegCodec::variableLenghtHuffmanCoding(int *input, int size, BitDataBuilder &builder)
{
    for (int i = 1; i < size; i += 2) {
        const int zeros = input[i];
        const int len = codeLength(input[i + 1]);
        builder.appendData(acHuffmanTable[(zeros << 4) + len], acHuffmanLength[(zeros << 4) + len]);
        int data =  input[i + 1];
        if (data < 0)
            data = ~data;

        builder.appendData(data, len);
    }
}

void JpegCodec::writeJpegToFile(const char *filename, const std::pair<std::vector<int>, int> &data, int width, int height)
{
    std::ofstream out(filename, std::ios_base::out | std::ios_base::binary);
    out << char(0xff) << char(0xd8); // SOI
    out << char(0xff) << char(0xe0); // APP
    out << char(0x00) << char(0x10);// app payload section

    // payload - jfif header
    out << char(0x4A) << char(0x46) << char(0x49) << char(0x46) << char(0x00) << char(0x01) << char(0x01)
        << char(0x00) << char(0x00) << char(0x01) << char(0x00) << char(0x01) << char(0x00) << char(0x00);

    out << char(0xff) << char(0xdb); // define quantization table
    out << char(0x00) << char(0x43); // 64 bytes quant table + 3 aux data
    out << char(0x00); // 4bits - qtable number + 4bits qtable precision

    for (int i = 0; i < 8 * 8; ++i)
        out << char(quantTable[zigZagOrder[i]]);

    out << char(0xff) << char(0xc0); // SOF
    out << char(0x00) << char(0x0b);
    out << char(0x08); // 8bit sample
    out << char(0x00) << char(height); // height
    out << char(0x00) << char(width); // width
    out << char(0x01); // only gray channel
    out << char(0x01) << char(0x11) << char(0x00); // gray channel data*/

    // DC Huffman table

    out << char(0xff) << char(0xc4); // DHT
    out << char(0x00) << char(sizeof(dcHuffmanNodes) - 1 + sizeof(dcHuffmanValues) + 3); // 31 byte follow
    out << char(0x00); // HT info: 0..3bits - HT number, 4 bit - HT type, 5-7 must be 0

    for (int i = 1; i < sizeof(dcHuffmanNodes); ++i)
        out << dcHuffmanNodes[i];

    for (int i = 0; i < sizeof(dcHuffmanValues); ++i)
        out << dcHuffmanValues[i];

    // AC Huffman table

    out << char(0xff) << char(0xc4); // DHT
    out << char(0x00) << char(sizeof(acHuffmanNodes) - 1 + sizeof(acHuffmanValues) + 3); // 31 byte follow TODO: maybe overflow
    out << char(0x10); // HT info: 0..3bits - HT number, 4 bit - HT type, 5-7 must be 0

    for (int i = 1; i < sizeof(acHuffmanNodes); ++i)
        out << acHuffmanNodes[i];

    for (int i = 0; i < sizeof(acHuffmanValues); ++i)
        out << acHuffmanValues[i];

    // start of scan
    out << char(0xff) << char(0xda);

    out << char(0x00) << char(0x08) << char(0x01) << char(0x01) << char(0x00);
    out << char(0x00) << char(0x3f) << char(0x00);

    for (int i = 0; i + 1 < data.first.size(); ++i) {
        const int x = data.first[i];

        char buffer[4];
        memcpy(buffer, &x, sizeof(int));

        for (int i = sizeof(buffer) - 1; i >= 0; --i) {
            out << buffer[i];
            std::cout << int((unsigned char)buffer[i]) << " ";

            if (buffer[i] == char(0xff)) {
                out << char(0x00);
            }
        }
    }

    const int diff = ceil((data.second - (data.first.size() - 1) * sizeof(int) * 8) / 8.0);
    const int intData = data.first[data.first.size() - 1];
    const unsigned char tailMask = data.second % 8 == 0 ? 0 : (1 << (8 - data.second % 8)) - 1;

    for (int i = 0; i < diff; ++i) {
        char c[sizeof(int)];
        memcpy(c, &intData, sizeof(int));

        char v = c[sizeof(int) - i - 1];

        if (i + 1 == diff)
            v |= tailMask;
        out << v;

        if (v == char(0xff))
            out << char(0x00);
    }

    out << char(0xff) << char(0xd9);
    out.close();
}

/*
 *
 *
 *
 */
