#include "vSeaskyPort/Protocol/CRC_Protocol/bsp_crc16.h"


static uint8_t          crc_tab16_init = 0;
static uint16_t         crc_tab16[256];

/*
 * uint16_t crc_16( const unsigned char *input_str, size_t num_bytes );
 *
 *函数crc_16()一次计算一个字节的16位CRC16
 *其开头已传递给函数的字符串。的数量
 *要检查的字节也是一个参数。字符串中的字节数为
 *受恒定大小最大值的限制。
 */
uint16_t crc_16(const uint8_t *input_str,uint16_t num_bytes )
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;
    if (!crc_tab16_init)init_crc16_tab();
    crc = CRC_START_16;
    ptr = input_str;
    if ( ptr != NULL ) for (a=0; a<num_bytes; a++)
            {
                crc = (crc >> 8) ^ crc_tab16[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
            }
    return crc;
}

/*
 * uint16_t crc_modbus( const unsigned char *input_str, size_t num_bytes );
 *
 *函数crc_modbus()一次计算16位modbus循环冗余校验
 *一个字节字符串，其开头已被传递给函数。这
 *要检查的字节数也是一个参数。
 */

uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes )
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;

    if ( ! crc_tab16_init ) init_crc16_tab();

    crc = CRC_START_MODBUS;
    ptr = input_str;
    if ( ptr != NULL ) for (a=0; a<num_bytes; a++)
            {

                crc = (crc >> 8) ^ crc_tab16[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
            }
    return crc;
}

/*
 * uint16_t update_crc_16( uint16_t crc, unsigned char c );
 *
 *函数update_crc_16()根据
 *前一个循环冗余校验值和下一个要检查的数据字节。
 */
uint16_t update_crc_16(uint16_t crc,uint8_t c )
{
    if ( ! crc_tab16_init ) init_crc16_tab();
    return (crc >> 8) ^ crc_tab16[ (crc ^ (uint16_t) c) & 0x00FF ];
}

/*
 * static void init_crc16_tab( void );
 *
 *为了获得最佳性能，使用CRC16例程查找带有值的表
 *可以直接在异或算法中使用的算法。
 *查找表首次由init_crc16_tab()例程计算
 *调用循环冗余校验函数。
 */
void init_crc16_tab(void)
{
    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;
    for (i=0; i<256; i++)
        {
            crc = 0;
            c   = i;
            for (j=0; j<8; j++)
                {
                    if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_16;
                    else                      crc =   crc >> 1;
                    c = c >> 1;
                }
            crc_tab16[i] = crc;
        }
    crc_tab16_init = 1;
}
