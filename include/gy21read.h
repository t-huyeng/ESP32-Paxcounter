#if (HAS_GY21)

#include "globals.h"

const int GY21_ADDRESS = 0x40;

bool gy21_init(const int addr);
double gy21_read_temperature(const int addr);
double gy21_read_humidity(const int addr);

#endif