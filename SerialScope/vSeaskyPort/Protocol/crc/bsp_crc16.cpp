/*
    @SEASKY---2022/03/23
*/
#include "bsp_crc16.h"

static uint8_t crc_tab16_init = 0;
static uint16_t crc_tab16[256];

/// <summary>
/// 函数crc_16()一次计算一个字节的16位CRC16
/// </summary>
/// <param name="input_str">字符串</param>
/// <param name="num_bytes">字节数</param>
/// <returns></returns>
uint16_t crc_16(const uint8_t *input_str, uint16_t num_bytes)
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;
    if (!crc_tab16_init)
        init_crc16_tab();
    crc = CRC_START_16;
    ptr = input_str;
    if (ptr != NULL)
        for (a = 0; a < num_bytes; a++)
        {
            crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
        }
    return crc;
}

/// <summary>
/// 一次计算16位modbus循环冗余校验
/// </summary>
/// <param name="input_str">字符串</param>
/// <param name="num_bytes">字节数</param>
/// <returns></returns>
uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes)
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;

    if (!crc_tab16_init)
        init_crc16_tab();

    crc = CRC_START_MODBUS;
    ptr = input_str;
    if (ptr != NULL)
        for (a = 0; a < num_bytes; a++)
        {

            crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
        }
    return crc;
}

/// <summary>
/// 前一个循环冗余校验值和下一个要检查的数据字节。
/// </summary>
/// <param name="crc"></param>
/// <param name="c"></param>
/// <returns></returns>
uint16_t update_crc_16(uint16_t crc, uint8_t c)
{
    if (!crc_tab16_init)
        init_crc16_tab();
    return (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)c) & 0x00FF];
}

/// <summary>
/// 为了获得最佳性能，使用CRC16例程查找带有值的表
/// </summary>
/// <param name=""></param>
void init_crc16_tab(void)
{
    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;
    if (crc_tab16_init == 0)
    {
        for (i = 0; i < 256; i++)
        {
            crc = 0;
            c = i;
            for (j = 0; j < 8; j++)
            {
                if ((crc ^ c) & 0x0001)
                    crc = (crc >> 1) ^ CRC_POLY_16;
                else
                    crc = crc >> 1;
                c = c >> 1;
            }
            crc_tab16[i] = crc;
        }
        crc_tab16_init = 1;
    }
}
