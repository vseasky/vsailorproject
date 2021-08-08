#ifndef _BSP_PROTOCOL_H
#define _BSP_PROTOCOL_H

#include <iostream>
#include <QString>

#define PROTOCOL_CMD_ID 0XA5    //定义开始帧头
#define OFFSET_BYTE 8           //除数据段外，其他部分所占字节数

typedef struct
{
    struct
    {
        uint8_t  sof = 0;
        uint16_t data_length = 0;
        uint8_t  crc_check = 0;     //帧头CRC校验
    } header;                       //数据帧头
    uint16_t cmd_id = 0;	 		//数据ID
    uint16_t frame_tail = 0; 		//帧尾CRC校验
} protocol;

typedef struct
{
    uint16_t cmd_id = 0;
    uint16_t flags_register = 0;
    float   *data;
    uint8_t  float_len = 0;//float有效数据长度
    uint8_t  *utf8_data;   //uint8_t有效数据长度
    uint16_t utf8_data_len = 0;
}protocol_info;


class SerialProtocol
{
public:
    SerialProtocol();

    protocol_info tx_info;
    protocol_info rx_info;
    protocol rx_pro;
    uint16_t rx_date_length;

    /*更新发送数据帧，并计算发送数据帧长度*/
    void get_protocol_send_data
    (uint16_t send_id,	 	  //信号id
     uint16_t flags_register, //16位寄存器
     float    *tx_data,		  //待发送的float数据
     uint8_t  float_length,   //float的数据长度
     uint8_t  *tx_buf,		  //待发送的数据帧
     uint16_t *tx_buf_len);	  //待发送的数据帧长度

     bool get_protocol_len
    (uint8_t  *rx_buf,			//接收到的原始数据
     uint16_t *rx_pos,			//原始数据指针
     uint16_t *thisdataLength);	//需要接收的数据总长度

    /*接收数据处理*/
    bool get_protocol_info
    (uint8_t  *rx_buf,			//接收到的原始数据
     uint16_t *rx_pos,			//原始数据指针
     uint16_t *flags_register,	//接收数据的16位寄存器地址
     float    *rx_data);		//接收的float数据存储地址
};

#endif
