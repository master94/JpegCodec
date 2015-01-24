#include "bitdatabuilder.h"

#include <iostream>

BitDataBuilder::BitDataBuilder()
    : m_currPos(0)
{
    m_data.resize(1);
}

void BitDataBuilder::appendBit(bool bit)
{
    if (bit) {
        m_data[m_data.size() - 1] |= 1 << (8 * sizeof(int) - 1 - m_currPos);
    }

    incrementCounter(1);
}

void BitDataBuilder::appendData(int data, int qty)
{
    if (qty == 0)
        return;

    const int rem = 8 * sizeof(int) - m_currPos;
    if (qty > rem) {
        int tmp = data >> (qty - rem);
        m_data[m_data.size() - 1] |= tmp;
        const int newQty = qty - rem;
        incrementCounter(rem);

        tmp = data << (8 * sizeof(int) - newQty);
        m_data[m_data.size() - 1] |= tmp;
        incrementCounter(newQty);
    } else {
        const int shift = 8 * sizeof(int) - m_currPos - qty;
        const int cleanMask = (1 << qty) - 1;
        m_data[m_data.size() - 1] |= (data & cleanMask) << shift;
        incrementCounter(qty);
    }
}

std::pair<std::vector<int>, int> BitDataBuilder::data()
{
    return std::make_pair(m_data, (m_data.size() - 1) * sizeof(int) * 8 + m_currPos);
}

void BitDataBuilder::incrementCounter(int qty)
{
    m_currPos += qty;

    if (m_currPos == 8 * sizeof(int)) {
        m_data.push_back(0);
        m_currPos = 0;
    }
}
