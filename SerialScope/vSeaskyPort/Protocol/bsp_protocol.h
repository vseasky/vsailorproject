#ifndef _BSP_PROTOCOL_H_
#define _BSP_PROTOCOL_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define PROTOCOL_HEAD_ID        0XA5
#define PROTOCOL_HEAD_LEN       4
#define PROTOCOL_DATA_OFFSET    10
#define MAX_BUFFER_SIZE         128


#define LOG_LEVEL_DEBUG (1<<0)
#define LOG_LEVEL_INFO_ (1<<1)
#define LOG_LEVEL_WARN_ (1<<2)
#define LOG_LEVEL_ERROR (1<<3)
#define LOG_LEVEL_FATAL (1<<4)

    typedef enum
    {
        PROTOCOL_RESULT_OK = 0,
        PROTOCOL_RESULT_ERR = 1,
        PROTOCOL_RESULT_CHECK_HEAD_ERR = 2,
        PROTOCOL_RESULT_CHECK_FRAME_ERR = 3,
        PROTOCOL_RESULT_OUT_OF_LEN = 4,
    }protocol_result;
    //32位
    typedef union
    {
        char        data_char[4];
        uint8_t     data_u8[4];
        int16_t     data_int16[2];
        uint16_t    data_u16[2];
        int32_t     data_int32;
        uint32_t    data_u32;
        float       data_float;
    }data_union;
    typedef  struct
    {
        /*字节偏移 4 -->> 字节大小 2*/
        uint16_t    equipment_type; //设备类型
        /*字节偏移 6 -->> 字节大小 2*/
        uint16_t    equipment_id;	//设备ID
        /*字节偏移 8 -->> 字节大小 data_len+2 */
        uint16_t    data_id;        //数据ID
        struct
        {
            uint16_t    data_len;     //data_union数据长度
            uint16_t    max_data_len; //数据最长长度
            uint32_t*   pData;        //数据(data_union),推荐使用 data_union * pDataUnion = &pData[];
        }cmd_data;
    } user_data_struct;//数据内容

    typedef struct
    {
        uint8_t*    pData;        //数据
        uint16_t    data_len;     //数据总长度
        uint16_t    max_data_len; //数据最长长度
    } message_struct;//消息数据

    typedef struct
    {
        /*字节偏移 0 -->> 字节大小 4*/
        struct
        {
            uint8_t  sof;           //固定帧头
            uint16_t data_length;   //数据长度
            uint8_t  crc_check;     //帧头CRC校验
        } header;					//数据帧头
        user_data_struct frame_user;//用户数据
        uint16_t         frame_tail;//帧尾CRC校验
    } frame_struct;

    typedef struct
    {
        frame_struct       frame_st;    //原始数据或解析后的数据
        message_struct     message_st;  //有待解析或生成的数据
    } protocol_struct;



    /// <summary>
    /// 根据data_union长度计算数据帧长度，用于分配内存
    /// </summary>
    /// <param name="uLen"></param>
    /// <returns></returns>
    uint16_t get_protocol_size(uint16_t uLen);

    /// <summary>
    /// 方式一 初始化,并初始化内存
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <param name="uLen"></param>
    void init_protocol(protocol_struct* pProtocol, uint16_t uLen);

    /// <summary>
    /// 方式二 外部预先分配好内存空间
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <param name="pFrameSt"></param>
    /// <param name="pMessageSt"></param>
    /// <param name="uLen"></param>
    /// <returns></returns>
    int init_protocol_pointer(protocol_struct* pProtocol, void* pFrameSt, void* pMessageSt, uint16_t uLen);

    /// <summary>
    /// 生成带发送的数据内容
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <returns></returns>
    int make_protocol(protocol_struct* pProtocol);

    /// <summary>
    /// 解析接收到的数据
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <returns></returns>
    int parse_protocol(protocol_struct* pProtocol,uint16_t parseDataLen);

    /// <summary>
    /// 反初始化，释放内存,如果是方式二创建然后反初始化，请注意规避野指针
    /// </summary>
    /// <param name="pProtocol"></param>
    void deinit_protocol(protocol_struct* pProtocol);



    /// <summary>
    /// 提供给用户，直接操作数据，但是请注意不要超过数据长度，避免内存越界
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <returns></returns>
    const user_data_struct* get_user_data_point(protocol_struct* pProtocol);


    /// <summary>
    /// 提供给用户，直接操作数据，但是请注意不要超过数据长度，避免内存越界
    /// </summary>
    /// <param name="pProtocol"></param>
    /// <returns></returns>
    const message_struct* get_message_point(protocol_struct* pProtocol);

#ifdef __cplusplus
}
#endif

#endif
