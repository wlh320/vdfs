//determine the position of bit
unsigned int bitPosition(unsigned int index)
{
    return (0x01 << index);
}

//get the value of bit
unsigned int getBit(unsigned int* value, int bitPosition)
{
    return (*value & bitPosition);
}

//set the value of bit
void setBit(unsigned int* value, int bitPosition)
{
    *value |= bitPosition;
}

//clear the bit
void clearBit(unsigned int* value, int bitPosition)
{
    *value &= ~(bitPosition);
}
