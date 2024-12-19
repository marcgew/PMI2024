#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

#define DS_SKIPROM  0xCC
#define DS_CONVERT_T 0x44
#define DS_READ_SCRATCHPAD 0xBE


void DS18B20_init(void);
uint32_t DS18B20_get_temp(void);


#endif /*DS18B20_H*/