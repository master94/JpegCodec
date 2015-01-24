#include "bitmap.h"

Bitmap::Bitmap(int w, int h)
    : m_data(nullptr)
    , m_width(0)
    , m_height(0)
{
    if (w <= 0 || h <= 0)
        return;

    m_data = new Pixel[w * h];
    m_width = w;
    m_height = h;
}

Bitmap::~Bitmap()
{
    delete[] m_data;
}

int Bitmap::width() const
{
    return m_width;
}

int Bitmap::height() const
{
    return m_height;
}

void Bitmap::setPixel(int x, int y, const Pixel &pixel)
{
    if (!isValid() || x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;

    m_data[x * m_width + y] = pixel;
}

Pixel Bitmap::pixel(int x, int y) const
{
    if (!isValid() || x < 0 || x >= m_width || y < 0 || y >= m_height)
        return Pixel();

    return m_data[x * m_width + y];
}

bool Bitmap::isValid() const
{
    return m_data != nullptr;
}

const Pixel *Bitmap::data() const
{
    return m_data;
}
