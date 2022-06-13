/*
	@SEASKY---2022/03/23
*/
#include <stdio.h>
#ifndef __MICROLIB 
#include <malloc.h >
#include <memory.h>
#include "bsp_protocol_class.h"
#endif
#include "bsp_protocol.h"
#include "./crc/bsp_crc8.h"
#include "./crc/bsp_crc16.h"


#ifdef PROTOCOL_CPP_CLR_DEBUG

#elif PROTOCOL_C_DEBUG
#define bsp_debug_c(Format, ...)   printf(Format, ##__VA_ARGS__)
#else
#define bsp_debug_c(...) 
#endif // !CPP_DEBUG

#define PROTOCOL_DEBUG_PRINTF(Format,...)         bsp_debug_c(LOG_LEVEL_DEBUG,Format, ##__VA_ARGS__)
#define PROTOCOL_INFO__PRINTF(Format,...)         bsp_debug_c(LOG_LEVEL_INFO_,Format, ##__VA_ARGS__)
#define PROTOCOL_WARN__PRINTF(Format,...)         bsp_debug_c(LOG_LEVEL_WARN_,Format, ##__VA_ARGS__)
#define PROTOCOL_ERROR_PRINTF(Format,...)         bsp_debug_c(LOG_LEVEL_ERROR,Format, ##__VA_ARGS__)
#define PROTOCOL_FATAL_PRINTF(Format,...)         bsp_debug_c(LOG_LEVEL_FATAL,Format, ##__VA_ARGS__)

/// <summary>
/// 获取CRC8校验码
/// </summary>
/// <param name="pchMessage"></param>
/// <param name="dwLength"></param>
/// <returns></returns>
static uint8_t Get_CRC8_Check(uint8_t *pchMessage,uint16_t dwLength)
{
    return crc_8(pchMessage,dwLength);
}

/// <summary>
/// 检验CRC8数据段
/// </summary>
/// <param name="pchMessage"></param>
/// <param name="dwLength"></param>
/// <returns></returns>
static uint8_t CRC8_Check_Sum(uint8_t *pchMessage,uint16_t dwLength)
{
    uint8_t ucExpected = 0;
    if ((pchMessage == 0) || (dwLength <= 2)) return 0;
    ucExpected = Get_CRC8_Check(pchMessage, dwLength-1);
    return (ucExpected == pchMessage[dwLength-1] );
}


/// <summary>
/// 获取CRC16校验码
/// </summary>
/// <param name="pchMessage"></param>
/// <param name="dwLength"></param>
/// <returns></returns>
static uint16_t Get_CRC16_Check(uint8_t *pchMessage,uint32_t dwLength)
{
    return crc_16(pchMessage,dwLength);
}

/// <summary>
/// 检验CRC16数据段
/// </summary>
/// <param name="pchMessage"></param>
/// <param name="dwLength"></param>
/// <returns></returns>
static uint16_t CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength)
{
    uint16_t  wExpected = 0;
    if ((pchMessage == 0) || (dwLength <= 2))
        {
            return 0;
        }
    wExpected = Get_CRC16_Check ( pchMessage, dwLength - 2);
    return (((wExpected & 0xff) == pchMessage[dwLength - 2] )&& (((wExpected >> 8) & 0xff) == pchMessage[dwLength - 1]));
}

/// <summary>
/// 检查帧头
/// </summary>
/// <param name="pData"></param>
/// <returns></returns>
static int check_protocol_heade(uint8_t *pData)
{
    if(pData[0] == PROTOCOL_HEAD_ID)
    {
        if(CRC8_Check_Sum(&pData[0],4))
            {
                return  PROTOCOL_RESULT_OK;
            }
    }
    return PROTOCOL_RESULT_CHECK_HEAD_ERR;
}

/// <summary>
/// 方式一->初始化frame_struct 自动分配地址
/// </summary>
/// <param name="pFrameStruct"></param>
/// <param name="uLen"></param>
/// <returns></returns>
static int init_frame_struct(frame_struct* pFrameStruct,uint16_t uLen)
{
    user_data_struct *pUserDataStruct = &pFrameStruct->frame_user;
    if ((pUserDataStruct->cmd_data.pData == NULL))
    {
        if (uLen <= MAX_BUFFER_SIZE)
        {
            pUserDataStruct->cmd_data.max_data_len = uLen;
            pUserDataStruct->cmd_data.pData = (uint32_t*) malloc(pUserDataStruct->cmd_data.max_data_len * (sizeof(uint32_t)));
            memset(pUserDataStruct->cmd_data.pData, 0, pUserDataStruct->cmd_data.max_data_len * (sizeof(data_union)));
            return PROTOCOL_RESULT_OK;
        }
        else
        {
            return PROTOCOL_RESULT_OUT_OF_LEN;
        }
    }
    else
    {
        return PROTOCOL_RESULT_ERR;
    }
}

/// <summary>
/// 方式二->初始化frame_struct 外部分配地址 该方式更方便修改数据
/// </summary>
/// <param name="pFrameStruct"></param>
/// <param name="pFrameSt"></param>
/// <param name="uLen"></param>
/// <returns></returns>
static int init_frame_pointer(frame_struct* pFrameStruct,void *pFrameSt,uint16_t uLen)
{
    user_data_struct *pUserDataStruct = &pFrameStruct->frame_user;
    if ((pUserDataStruct->cmd_data.pData == NULL))
    {
        if (uLen <= MAX_BUFFER_SIZE)
        {
            pUserDataStruct->cmd_data.max_data_len = uLen;
            pUserDataStruct->cmd_data.pData = (uint32_t *)pFrameSt;
            // pUserDataStruct->cmd_data.pData = (uint32_t*) malloc(pUserDataStruct->cmd_data.max_data_len * (sizeof(uint32_t)));
            memset(pUserDataStruct->cmd_data.pData, 0, pUserDataStruct->cmd_data.max_data_len * (sizeof(data_union)));
            return PROTOCOL_RESULT_OK;
        }
        else
        {
            return PROTOCOL_RESULT_OUT_OF_LEN;
        }
    }
    else
    {
        return PROTOCOL_RESULT_ERR;
    }
}

/// <summary>
/// 方式一 ->初始化message_struct
/// </summary>
/// <param name="pMessageStruct"></param>
/// <param name="uLen"></param>
/// <returns></returns>
static int init_message_struct(message_struct* pMessageStruct,uint16_t uLen)
{
    if ((pMessageStruct->pData == NULL))
    {
        if (uLen <= get_protocol_size(MAX_BUFFER_SIZE))
        {
            pMessageStruct->max_data_len = uLen;
            pMessageStruct->pData = (uint8_t *)malloc(pMessageStruct->max_data_len * (sizeof(uint8_t)));
            memset(pMessageStruct->pData, 0, pMessageStruct->max_data_len * (sizeof(uint8_t)));
            return PROTOCOL_RESULT_OK;
        }
        else
        {
            return PROTOCOL_RESULT_OUT_OF_LEN;
        }
    }
    else
    {
        return PROTOCOL_RESULT_ERR;
    }
}

/// <summary>
/// 方式二 ->初始化message_struct
/// </summary>
/// <param name="pMessageStruct"></param>
/// <param name="pMessageSt"></param>
/// <param name="uLen"></param>
/// <returns></returns>
static int init_message_pointer(message_struct* pMessageStruct,void *pMessageSt,uint16_t uLen)
{
    if ((pMessageStruct->pData == NULL))
    {
        if (uLen <= get_protocol_size(MAX_BUFFER_SIZE))
        {
            pMessageStruct->max_data_len = uLen;
            pMessageStruct->pData = (uint8_t*)pMessageSt;
            // pMessageStruct->pData = (uint8_t *)malloc(pMessageStruct->max_data_len * (sizeof(uint8_t)));
            memset(pMessageStruct->pData, 0, pMessageStruct->max_data_len * (sizeof(uint8_t)));
            return PROTOCOL_RESULT_OK;
        }
        else
        {
            return PROTOCOL_RESULT_OUT_OF_LEN;
        }
    }
    else
    {
        return PROTOCOL_RESULT_ERR;
    }
}

/// <summary>
/// 根据data_union长度计算数据帧长度，用于分配内存
/// </summary>
/// <param name="uLen"></param>
/// <returns></returns>
uint16_t get_protocol_size(uint16_t uLen)
{
    return (uLen * sizeof(uint32_t) + PROTOCOL_DATA_OFFSET + 2);
}

/// <summary>
/// 方式一 初始化,并初始化内存
/// </summary>
/// <param name="pProtocol"></param>
/// <param name="uLen"></param>
void init_protocol(protocol_struct* pProtocol, uint16_t uLen)
{
    /*初始化crc*/
    init_crc16_tab();
    /*分配内存空间*/
    if (init_frame_struct(&pProtocol->frame_st, uLen) != 0)
    {
        PROTOCOL_ERROR_PRINTF("pProtocol init_frame_struct err!\n");
    }
    PROTOCOL_DEBUG_PRINTF("pProtocol init_frame_struct ok!\n");
    if (init_message_struct(&pProtocol->message_st, get_protocol_size(uLen)) != 0)
    {
        PROTOCOL_ERROR_PRINTF("pProtocol init_message_struct err!\n");
    }
    PROTOCOL_DEBUG_PRINTF("pProtocol init_message_struct ok!\n");
}

/// <summary>
/// 方式二 外部预先分配好内存空间
/// </summary>
/// <param name="pProtocol"></param>
/// <param name="pFrameSt"></param>
/// <param name="pMessageSt"></param>
/// <param name="uLen"></param>
/// <returns></returns>
int init_protocol_pointer(protocol_struct* pProtocol, void *pFrameSt,void *pMessageSt,uint16_t uLen)
{
    /*初始化crc*/
    init_crc16_tab();
    /*分配内存空间*/
    if (init_frame_pointer(&pProtocol->frame_st,pFrameSt, uLen) != 0)
    {
        PROTOCOL_ERROR_PRINTF("pProtocol init_frame_struct err!\n"); 
        return PROTOCOL_RESULT_ERR;
    }
    PROTOCOL_DEBUG_PRINTF("pProtocol init_frame_struct ok!\n");
    if (init_message_pointer(&pProtocol->message_st,pMessageSt,get_protocol_size(uLen)) != 0)
    {
        PROTOCOL_ERROR_PRINTF("pProtocol init_message_struct err!\n");
        return PROTOCOL_RESULT_ERR;
    }
    PROTOCOL_DEBUG_PRINTF("pProtocol init_message_struct ok!\n");
    return PROTOCOL_RESULT_OK;
}

/// <summary>
/// 生成带发送的数据内容
/// </summary>
/// <param name="pProtocol"></param>
/// <returns></returns>
int make_protocol(protocol_struct* pProtocol)
{
    int ret = -1;
    uint16_t pos_offset;
    frame_struct    *pFrameStruct   = &pProtocol->frame_st;
    message_struct  *pMessageStruct = &pProtocol->message_st;
    //更新头部数据
    pFrameStruct->header.sof         = PROTOCOL_HEAD_ID;
    pFrameStruct->header.data_length = pFrameStruct->frame_user.cmd_data.data_len*sizeof(data_union);
    
    pMessageStruct->pData[0] = pFrameStruct->header.sof;
    pMessageStruct->pData[1] = (pFrameStruct->header.data_length)&0xff;
    pMessageStruct->pData[2] = (pFrameStruct->header.data_length>>8)&0xff;

    pFrameStruct->header.crc_check   =  Get_CRC8_Check(&pMessageStruct->pData[0],3);
    pMessageStruct->pData[3] = pFrameStruct->header.crc_check;

    //更新设备类型
    pMessageStruct->pData[4] = (pFrameStruct->frame_user.equipment_type)&0xff;
    pMessageStruct->pData[5] = (pFrameStruct->frame_user.equipment_type>>8)&0xff;
    //更新设备ID
    pMessageStruct->pData[6] = (pFrameStruct->frame_user.equipment_id)&0xff;
    pMessageStruct->pData[7] = (pFrameStruct->frame_user.equipment_id >>8)&0xff;
    //更新设备ID
    pMessageStruct->pData[8] = (pFrameStruct->frame_user.data_id) & 0xff;
    pMessageStruct->pData[9] = (pFrameStruct->frame_user.data_id >> 8) & 0xff;
    if (pFrameStruct->frame_user.cmd_data.data_len <= pFrameStruct->frame_user.cmd_data.max_data_len)
    {
        pos_offset = pFrameStruct->frame_user.cmd_data.data_len * sizeof(data_union);
        //复制内存
        ret = (int)memcpy(&pMessageStruct->pData[PROTOCOL_DATA_OFFSET],&pFrameStruct->frame_user.cmd_data.pData[0], pos_offset);
        pos_offset = pFrameStruct->frame_user.cmd_data.data_len * sizeof(data_union)+PROTOCOL_DATA_OFFSET;
        //帧尾校验值
        pFrameStruct->frame_tail = Get_CRC16_Check(&pMessageStruct->pData[0], pos_offset);
        pMessageStruct->pData[pos_offset] = (pFrameStruct->frame_tail)&0XFF;
        pMessageStruct->pData[pos_offset+1] = (pFrameStruct->frame_tail>>8) & 0XFF;
        pMessageStruct->data_len = pos_offset + 2;
        PROTOCOL_DEBUG_PRINTF("make_protocol->>data_len[%d] > max_data_len[%d]!\n",
            pFrameStruct->frame_user.cmd_data.data_len,
            pFrameStruct->frame_user.cmd_data.max_data_len);
        return PROTOCOL_RESULT_OK;
    }
    else
    {
        PROTOCOL_ERROR_PRINTF("make_protocol->>data_len[%d] > max_data_len[%d]!\n",
                pFrameStruct->frame_user.cmd_data.data_len, 
                pFrameStruct->frame_user.cmd_data.max_data_len);
        return PROTOCOL_RESULT_OUT_OF_LEN;
    }
    PROTOCOL_DEBUG_PRINTF("make_protocol->>data_len[%d],max_data_len[%d]!\n",
        pFrameStruct->frame_user.cmd_data.data_len,
        pFrameStruct->frame_user.cmd_data.max_data_len);
    return PROTOCOL_RESULT_ERR;
}

/// <summary>
/// 解析接收到的数据
/// </summary>
/// <param name="pProtocol"></param>
/// <returns></returns>
int parse_protocol(protocol_struct* pProtocol,uint16_t parseDataLen)
{
    //解析数据，使用前需提前缓冲 pProtocol->message_st.pData
    int ret = -1;
    uint16_t pos_offset;
    frame_struct* pFrameStruct = &pProtocol->frame_st;
    message_struct* pMessageStruct = &pProtocol->message_st;
    if (check_protocol_heade(pMessageStruct->pData) == PROTOCOL_RESULT_OK)
    {
        //更新帧头数据
        pFrameStruct->header.sof            = pMessageStruct->pData[0];
        //获取data段的数据长度
        pFrameStruct->header.data_length    = (pMessageStruct->pData[2] << 8) | (pMessageStruct->pData[1]);
        pFrameStruct->header.crc_check      = pMessageStruct->pData[3];

        //获取此次数据包长度
        pMessageStruct->data_len = pFrameStruct->header.data_length + PROTOCOL_DATA_OFFSET + 2;
        //计算解析后得到的 data_union 数据长度
        pFrameStruct->frame_user.cmd_data.data_len = (pFrameStruct->header.data_length) / sizeof(data_union);
        if(pMessageStruct->data_len<=parseDataLen)
        {
            if (pMessageStruct->data_len <= pMessageStruct->max_data_len)
            {
                if(CRC16_Check_Sum(&pMessageStruct->pData[0], pMessageStruct->data_len) != 0)
                {
                    pFrameStruct->frame_user.equipment_type = (pMessageStruct->pData[5]<<8) | (pMessageStruct->pData[4]);
                    pFrameStruct->frame_user.equipment_id   = (pMessageStruct->pData[7] << 8) | (pMessageStruct->pData[6]);
                    pFrameStruct->frame_user.data_id        = (pMessageStruct->pData[9] << 8) | (pMessageStruct->pData[8]);
                    //拷贝 data段 指定长度数据
                    ret = (int)memcpy(&pFrameStruct->frame_user.cmd_data.pData[0], &pMessageStruct->pData[PROTOCOL_DATA_OFFSET], pFrameStruct->header.data_length);
                    pos_offset = pFrameStruct->header.data_length + PROTOCOL_DATA_OFFSET;
                    pFrameStruct->frame_tail = (pMessageStruct->pData[pos_offset+1] << 8) | (pMessageStruct->pData[pos_offset]);
                    return PROTOCOL_RESULT_OK;
                }
                else
                {
                    //待解析BUFF超过预定解析数据容量，避免内存越界
                    PROTOCOL_ERROR_PRINTF("parse_protocol->>CRC16_Check_Sum err!\n");
                    return PROTOCOL_RESULT_CHECK_FRAME_ERR;
                }
            }
            else
            {
                //待解析BUFF超过预定解析数据容量，避免内存越界
                PROTOCOL_ERROR_PRINTF("parse_protocol->>data_len[%d] > max_data_len[%d]!\n",
                    pMessageStruct->data_len,
                    pMessageStruct->max_data_len);
                return PROTOCOL_RESULT_OUT_OF_LEN;
            }
        }
        else
        {
            //通过包头计算，还未收到完整的数据包
//            PROTOCOL_ERROR_PRINTF("parse_protocol->>data_len[%d] > max_data_len[%d]!\n",
//                pMessageStruct->data_len,
//                pMessageStruct->max_data_len);
            return PROTOCOL_RESULT_OUT_OF_LEN;
        }
    }
    else
    {
        //待解析BUFF超过预定解析数据容量，避免内存越界
        PROTOCOL_ERROR_PRINTF("parse_protocol->>check_protocol_heade err!\n");
        return PROTOCOL_RESULT_CHECK_HEAD_ERR;
    }
    PROTOCOL_DEBUG_PRINTF("parse_protocol->>check_protocol_heade ok!\n");
    return PROTOCOL_RESULT_ERR;
}


/// <summary>
/// 反初始化，释放内存,如果是方式二创建然后反初始化，请注意规避野指针
/// </summary>
/// <param name="pProtocol"></param>
void deinit_protocol(protocol_struct* pProtocol)
{
    //指针为空不需要判断
    free(pProtocol->frame_st.frame_user.cmd_data.pData);
    free(pProtocol->message_st.pData);
    //memset(pProtocol, 0, sizeof(protocol_struct));
    pProtocol->frame_st.frame_user.cmd_data.pData = NULL;
    pProtocol->message_st.pData = NULL;
}

/// <summary>
/// 提供给用户，直接操作数据，但是请注意不要超过数据长度，避免内存越界
/// </summary>
/// <param name="pProtocol"></param>
/// <returns></returns>
const user_data_struct  * get_user_data_point(protocol_struct* pProtocol)
{
    return &pProtocol->frame_st.frame_user;
}

/// <summary>
/// 提供给用户，直接操作数据，但是请注意不要超过数据长度，避免内存越界
/// </summary>
/// <param name="pProtocol"></param>
/// <returns></returns>
const message_struct    * get_message_point(protocol_struct* pProtocol)
{
    return &pProtocol->message_st;
}


