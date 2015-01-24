#ifndef JPEGCODEC_H
#define JPEGCODEC_H

#include <vector>

class Bitmap;
class BitDataBuilder;

class JpegCodec
{
public:
    JpegCodec();

    static std::pair<std::vector<int>, int> encode(const Bitmap &bmp);
    static void writeJpegToFile(const char *filename, const std::pair<std::vector<int>, int> &data);

private:
    static void dct(char *data, int width, int height, BitDataBuilder &builder);
    static void dctBlock(const char *input, int *output, int blockSize);
    static void quantizeBlock(int *data, int blockSize);
    static double dctPixel(const char *data, int blockSize, int u, int v);
    static int runLengthEncoding(int *input, int *output, int blockSize);
    static void variableLenghtHuffmanCoding(int *input, int size, BitDataBuilder &builder);
};

#endif // JPEGCODEC_H
