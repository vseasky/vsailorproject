#ifndef _BSP_CRC16_H
#define _BSP_CRC16_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>


#define	CRC_START_16	 0xFFFF
#define	CRC_START_MODBUS 0xFFFF
#define	CRC_POLY_16		 0xA001


/// <summary>
/// 函数crc_16()一次计算一个字节的16位CRC16
/// </summary>
/// <param name="input_str">字符串</param>
/// <param name="num_bytes">字节数</param>
/// <returns></returns>
uint16_t crc_16(const uint8_t *input_str,uint16_t num_bytes );

/// <summary>
/// 一次计算16位modbus循环冗余校验
/// </summary>
/// <param name="input_str">字符串</param>
/// <param name="num_bytes">字节数</param>
/// <returns></returns>
uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes );

/// <summary>
/// 前一个循环冗余校验值和下一个要检查的数据字节。
/// </summary>
/// <param name="crc"></param>
/// <param name="c"></param>
/// <returns></returns>
uint16_t update_crc_16(uint16_t crc,uint8_t c );

/// <summary>
/// 为了获得最佳性能，使用CRC16例程查找带有值的表
/// </summary>
/// <param name=""></param>

void init_crc16_tab( void );


#ifdef __cplusplus
}
#endif
#endif
