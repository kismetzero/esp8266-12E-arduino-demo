#include "MatrixDisplay.h"

MatrixDisplay::MatrixDisplay(uint8_t numDevices):_maxDevices(numDevices)
{
    _helpArrMAX = new uint8_t[_maxDevices * 8];
    _helpArrPos = new uint8_t[_maxDevices * 8];
    for (uint8_t i = 0, j = 0, k = 0; i < _maxDevices * 8; i++)
    {
        _helpArrPos[i] = (1 << j); // bitmask
        _helpArrMAX[i] = k;
        j++;
        if (j > 7)
        {
            j = 0;
            k++;
        }
    }

    _LEDarr = new uint8_t *[_maxDevices];
    for (uint8_t i = 0; i < _maxDevices; i++)
        _LEDarr[i] = new uint8_t[8];
    clearBuffer();

    _maxPosX = _maxDevices * 8 - 1;
}

MatrixDisplay::~MatrixDisplay()
{
    delete[] _helpArrMAX;
    _helpArrMAX = nullptr;
    delete[] _helpArrPos;
    _helpArrPos = nullptr;

    for (uint8_t i = 0; i < _maxDevices; i++)
        delete[] _LEDarr[i];
    delete[] _LEDarr;
    _LEDarr = nullptr;
}

void MatrixDisplay::clearBuffer()
{
    for (uint8_t i = 0; i < _maxDevices; i++)
        for (uint8_t j = 0; j < 8; j++)
            _LEDarr[i][j] = 0;
}

void MatrixDisplay::drawBigChar(uint8_t ch, int32_t PosX, int16_t PosY)
{
    int32_t i, j, k, l, m, o1, o2, o3, o4; // in LEDarr
    PosX++;
    k = ch;
    if (ch > 24)
    k = ch - 25;              // ASCII position in font
    if ((k >= 0) && (k < 103)) // character found in font?
    {
        o4 = _bigFont[k][0]; // character width
        o3 = 1 << (o4 - 2);
        for (i = 0; i < o4; i++)
        {
            if (((PosX - i <= _maxPosX) && (PosX - i >= 0)) && ((PosY > -8) && (PosY < 8))) // within matrix?
            {
                o1 = _helpArrPos[PosX - i];
                o2 = _helpArrMAX[PosX - i];
                for (j = 0; j < 8; j++)
                {
                    if (((PosY >= 0) && (PosY <= j)) || ((PosY < 0) && (j < PosY + 8))) // scroll vertical
                    {
                        l = _bigFont[k][j + 1];
                        m = (l & (o3 >> i)); // e.g. o4=7  0zzzzz0, o4=4  0zz0
                        if (m > 0)
                            _LEDarr[o2][j - PosY] = _LEDarr[o2][j - PosY] | (o1); // set point
                        else
                            _LEDarr[o2][j - PosY] = _LEDarr[o2][j - PosY] & (~o1); // clear point
                    }
                }
            }
        }
    }
}

void MatrixDisplay::drawSmallChar(uint8_t ch, int32_t PosX, int16_t PosY)
{
    int i, j, k, l, m, o1, o2, o3, o4; // in LEDarr
    PosX++;
    k = ch;
    if (ch > 24)
    k = ch - 25;              // ASCII position in font
    if ((k >= 0) && (k < 103)) // character found in font?
    {
        o4 = _smallFont[k][0]; // character width
        o3 = 1 << (o4 - 2);
        for (i = 0; i < o4; i++)
        {
            if (((PosX - i <= _maxPosX) && (PosX - i >= 0)) && ((PosY > -8) && (PosY < 8))) // within matrix?
            {
                o1 = _helpArrPos[PosX - i];
                o2 = _helpArrMAX[PosX - i];
                for (j = 0; j < 8; j++)
                {
                    if (((PosY >= 0) && (PosY <= j)) || ((PosY < 0) && (j < PosY + 8))) // scroll vertical
                    {
                        l = _smallFont[k][j + 1];
                        m = (l & (o3 >> i)); // e.g. o4=7  0zzzzz0, o4=4  0zz0
                        if (m > 0)
                            _LEDarr[o2][j - PosY] = _LEDarr[o2][j - PosY] | (o1); // set point
                        else
                            _LEDarr[o2][j - PosY] = _LEDarr[o2][j - PosY] & (~o1); // clear point
                    }
                }
            }
        }
    }
}
