#ifndef BITDATABUILDER_H
#define BITDATABUILDER_H

#include <vector>

class BitDataBuilder
{
public:
    BitDataBuilder();

    void appendBit(bool bit);
    void appendData(int data, int qty);

    std::pair<std::vector<int>, int> data();

private:
    std::vector<int> m_data;
    int m_currPos;

    void incrementCounter(int qty);

};

#endif // BITDATABUILDER_H
