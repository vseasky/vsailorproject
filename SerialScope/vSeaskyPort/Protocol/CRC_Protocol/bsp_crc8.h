#ifndef _BSP_CRC8_H
#define _BSP_CRC8_H

#include <iostream>


#define	CRC_START_8	0x00

uint8_t crc_8( const uint8_t *input_str,uint16_t num_bytes);
uint8_t update_crc_8(uint8_t crc, uint8_t val );

#endif
