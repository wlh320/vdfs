#include <cstdint>

//set bitPostion in value to 1
void setBit(unsigned int* value, int bitPosition)
{
    *value |= bitPosition;
}

//set bitPostion in value to 0
void clearBit(unsigned int* value, int bitPosition)
{
    *value &= ~(bitPosition);
}

// 奇技淫巧返回第一个为0的位置
// http://www.matrix67.com/blog/archives/3985
int firstZeroPos(unsigned int x)
{
    static const int MultiplyDeBruijnBitPosition[32] =
    {
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };

    x = ~x;
    return MultiplyDeBruijnBitPosition[((uint32_t)((x & -x) * 0x077CB531U)) >> 27];
}
