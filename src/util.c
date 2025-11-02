#include "util.h"

void float_to_byte_array(float f, char *arr)
{
    unsigned int asInt = *((int*) &f);

    for (int i = 0; i < 4; i++)
        arr[i] = (asInt >> 8 * i) & 0xFF;
}

void float_16_to_byte_array(float f, char *arr)
{
    unsigned int asInt = *((int*) &f);

    for (int i = 0; i < 4; i++)
        arr[i] = (asInt >> 8 * i) & 0xFF;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

float map(int min, int max, int pos)
{
	return ((float) (pos - min)) / (max - min);
}