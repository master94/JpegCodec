#ifndef BITMAP_H
#define BITMAP_H

class Pixel {
public:
    Pixel()
        : m_red(0)
        , m_green(0)
        , m_blue(0) {}

    Pixel(unsigned char r, unsigned char g, unsigned char b)
        : m_red(r)
        , m_green(g)
        , m_blue(b) {}

    unsigned char red() const { return m_red; }
    unsigned char green() const { return m_green; }
    unsigned char blue() const { return m_blue; }

private:
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;

};

class Bitmap
{
public:
    Bitmap(int w, int h);
    ~Bitmap();

    int width() const;
    int height() const;

    void setPixel(int x, int y, const Pixel &pixel);
    Pixel pixel(int x, int y) const;

    bool isValid() const;

    const Pixel *data() const;

private:
    Pixel *m_data;
    int m_width;
    int m_height;

};

#endif // BITMAP_H
