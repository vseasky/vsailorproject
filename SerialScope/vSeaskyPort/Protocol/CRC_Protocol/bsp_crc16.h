#ifndef _BSP_CRC16_H
#define _BSP_CRC16_H

#include <iostream>

#define	CRC_START_16	 0xFFFF
#define	CRC_START_MODBUS 0xFFFF
#define	CRC_POLY_16		 0xA001

uint16_t crc_16(const uint8_t *input_str,uint16_t num_bytes );
uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes );
uint16_t update_crc_16(uint16_t crc,uint8_t c );
void init_crc16_tab( void );

#endif
