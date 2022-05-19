#ifndef _BSP_CRC8_H
#define _BSP_CRC8_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>


#define	CRC_START_8	0x00

/// <summary>
/// 计算输入字符串的8位宽crc
/// </summary>
/// <param name="input_str"></param>
/// <param name="num_bytes">给定长度</param>
/// <returns></returns>
uint8_t crc_8( const uint8_t *input_str,uint16_t num_bytes);

/// <summary>
/// 给定一个数据字节和循环冗余校验值的前一个值
/// </summary>
/// <param name="crc"></param>
/// <param name="val"></param>
/// <returns>计算并返回数据的新的实际crc值</returns>
uint8_t update_crc_8(uint8_t crc, uint8_t val );


#ifdef __cplusplus
}
#endif

#endif
