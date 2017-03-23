#ifndef UTILS_H
#define UTILS_H

typedef unsigned char byte;

#define YES 0
#define ERR 1

// bitmap functions
void setBit(unsigned int* value, int bitPosition);
void clearBit(unsigned int* value, int bitPosition);
int firstZeroPos(unsigned int x);


#endif //UTILS_H
