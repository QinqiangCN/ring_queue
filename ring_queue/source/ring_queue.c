/**
* @file       ring_queue.c
* @brief       环形队列
* @details     可插入不定长数据的环形队列
* @author      Qinqiang
* @date        2021-7-28
* @version     V1.0
* @copyright   Copyright (c) 2021
**********************************************************************************
* @attention
*
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/07/28  <td>1.0      <td>Qinqiang  <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#include <stdint.h>
#include <string.h>

#include "ring_queue.h"

static void ring_queue_statistical_information_updata(ring_queue_Type_Def* ring_queue_Struct);

/**
* @brief                            指针计算
*
* @param[in]  ring_queue_Struct     队列管理结构体指针
* @param[in]  point                 操作指针枚举
*   WRITE_POINT                     写指针操作
*   READ_POINT                      读指针操作
*   TEMP_POINT                      临时指针操作
*
*/
static void add_point(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_or_Read_t point)
{
    if (point == WRITE_POINT) {
        if (++ring_queue_Struct->store_w >= ring_queue_Struct->store_size) {
            ring_queue_Struct->store_w = 0;
        }
        /// 写入时读写指针重合，则缓存满
        if (ring_queue_Struct->store_w == ring_queue_Struct->store_r) {
            ring_queue_Struct->store_state = STORE_FULL;
        }
    }
    else if (point == READ_POINT) {
        if (++ring_queue_Struct->store_r >= ring_queue_Struct->store_size) {
            ring_queue_Struct->store_r = 0;
        }
        /// 读取时读写指针重合，则缓存空
        if (ring_queue_Struct->store_w == ring_queue_Struct->store_r) {
            ring_queue_Struct->store_state = STORE_EMPTY;
        }
    }
    else if (point == TEMP_POINT) {
        if (++ring_queue_Struct->store_temp >= ring_queue_Struct->store_size) {
            ring_queue_Struct->store_temp = 0;
        }
    }
}

/**
* @brief                            初始化队列
*
* @param[in]  *ring_queue_Struct    帧管理结构体指针
* @param[in]  *pBuffer              为队列分配空间指针
* @param[in]  buffer_size_byte      为队列分配空间长度(byte)
*
* @return                           函数执行结果
*   0                               成功
*   -1                              失败
* @par 示例:
* @code
*    ring_queue_Type_Def     uartRxBuf_Struct;
*    uint8_t                 uartRxBuf[233];
*    int ret = ring_queue_init(&uartRxBuf_Struct, uartRxBuf, sizeof(uartRxBuf));
* @endcode
*/
int ring_queue_init(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pBuffer, uint32_t buffer_size_byte)
{
    if (ring_queue_Struct == NULL || pBuffer == NULL || buffer_size_byte <= 0) {
        return -1;
    }
    if (buffer_size_byte > UINT32_MAX) {
        return -1;
    }
    ring_queue_Struct->pBuf = pBuffer;
    ring_queue_Struct->store_size = buffer_size_byte;
    ring_queue_Struct->store_w = 0;
    ring_queue_Struct->store_r = 0;
    ring_queue_Struct->store_temp = 0;
    ring_queue_Struct->store_state = STORE_EMPTY;
    ring_queue_Struct->store_free_space = buffer_size_byte;
    ring_queue_Struct->space_occupancy = 0;
    ring_queue_Struct->space_occupancy_max = 0;

    return 0;
}

/**
* @brief                            向队列写入一个字节
*
* @param[in]  *ring_queue_Struct    队列管理句柄
* @param[in]  byte                  待写入数据
*
* @return                           函数执行结果
*   0                               成功
*/
static int ring_queue_write_byte(ring_queue_Type_Def* ring_queue_Struct, uint8_t byte)
{
    /***
    旧数据被覆盖标志
    **/
    if (ring_queue_Struct->store_state == STORE_FULL) {
        ring_queue_Struct->store_state = STORE_FULL_DATA_OVERWRITTEN;
    }
    /// 字节写入
    ((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_w] = byte;
    /// 写指针移动
    add_point(ring_queue_Struct, WRITE_POINT);

    return 0;
}

/**
* @brief                            队列读取一个字节
*
* @param[in]  *ring_queue_Struct    队列管理句柄
*
* @return                           读取数据
*/
static uint8_t ring_queue_read_byte(ring_queue_Type_Def* ring_queue_Struct)
{
    uint8_t byte = 0xFF;

    /// 字节读取
    byte = ((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r];
    /// 读指针移动
    add_point(ring_queue_Struct, READ_POINT);

    return byte;
}

/**
* @brief                            向队列写入一帧数据
*
* @param[in]  *ring_queue_Struct    帧管理结构体指针
* @param[in]  *pData                待写入数据指针
* @param[in]  data_size_byte        待写入数据长度（byte）
*
* @return                           函数执行结果
*   0                               成功
*   -1                              失败
*/
int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pData, uint32_t data_size_byte)
{
    uint8_t data = 0;

    /***
    异常处理
    **/
    /// 空指针判断
    if ( (ring_queue_Struct == NULL) || (pData == NULL) ) {
        return -1;
    }
    /// 长度错误判断
    if ( (data_size_byte > ring_queue_Struct->store_size + 2) || (data_size_byte > UINT32_MAX) || (data_size_byte <= 0) ) {
        return -1;
    }
    /***
    Slip Code
    **/
    /// 帧头
    ring_queue_write_byte(ring_queue_Struct, SLIP_START);
    /// 遍历每个字节
    for (uint32_t i = 0; i < data_size_byte; i++) {
        data = *((uint8_t*)pData + i);
        if (data == SLIP_START) {
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC);
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC_START);
        }
        else if (data == SLIP_END) {
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC);
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC_END);
        }
        else if (data == SLIP_ESC) {
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC);
            ring_queue_write_byte(ring_queue_Struct, SLIP_ESC_ESC);
        }
        else {
            ring_queue_write_byte(ring_queue_Struct, data);
        }
    }
    /// 帧尾
    ring_queue_write_byte(ring_queue_Struct, SLIP_END);
    /// 帧状态
    if (ring_queue_Struct->store_state != STORE_FULL_DATA_OVERWRITTEN) {
        ring_queue_Struct->store_state = STORE_NORMAL;
    }
    /// 是否在队列旧数据被覆盖后立即开始寻找新的帧头，0：不启用，1：启用。
    ///（启用统计信息时需要启用该功能）
    ///（不启用该功能则单帧数据长度小于队列长度，而实际写入长度超过队列长度时依然返回写入成功）
    if ( ( ( 0 ) && (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN)) || (STATISTICAL_INFORMATION == 1) ) {
        /// 读指针移动到写指针位置
        ring_queue_Struct->store_r = ring_queue_Struct->store_w;
        /***
         查找帧头
         **/
        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// 读指针移动
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// 再次重合-无帧头
                ring_queue_Struct->store_state = STORE_EMPTY;
#if STATISTICAL_INFORMATION == 1
                /// 更新统计信息
                ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
                return -1;
            }
        }
        ring_queue_Struct->store_state = STORE_NORMAL;
    }

#if STATISTICAL_INFORMATION == 1
    /// 更新统计信息
    ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
    return 0;
}

/**
* @brief                            从队列读取一帧数据
*
* @param[in]  *ring_queue_Struct    帧管理结构体指针
* @param[out] *pData                读取数据被写入该地址
* @param[out] data_size_byte        读取帧长度被写入该地址
*
* @return                           函数执行结果
*   0                               成功
*   -1                              失败
*/
int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte)
{
    uint8_t data = 0;
    uint8_t ReadBuf[520] = { 0 };
    uint32_t buf_count = 0;
    uint32_t len = 0;

    /***
    异常处理
    **/
    /// 空指针判断
    if (ring_queue_Struct == NULL || pData == NULL) {
        return -1;
    }
    ///// 长度超限判断
    //if (data_size_byte > UINT32_MAX) {
    //    return -1;
    //}
    /// 队列空判断
    if (ring_queue_Struct->store_state == STORE_EMPTY) {
        return -1;
    }

    if (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN) {
        /// 读指针移动到写指针位置
        ring_queue_Struct->store_r = ring_queue_Struct->store_w;
        /***
        查找帧头
        **/
        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// 读指针移动
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// 再次重合-无帧头
                ring_queue_Struct->store_state = STORE_EMPTY;
                return -1;
            }
        }
    }
    /// 读指针移动-略过帧头
    add_point(ring_queue_Struct, READ_POINT);

    do {
        /***
        Slip Code
        **/
        /// 解析一个字节
        data = ring_queue_read_byte(ring_queue_Struct);
        if (data == SLIP_ESC) {
            ReadBuf[buf_count++] = ring_queue_read_byte(ring_queue_Struct) - 1;
        }
        else {
            ReadBuf[buf_count++] = data;
        }
        /// 长度计数
        len++;
        
        if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
            /// 再次重合-无帧尾
            ring_queue_Struct->store_state = STORE_EMPTY;
            return -1;
        }
    } while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_END);
    /// 读指针移动-略过帧尾
    add_point(ring_queue_Struct, READ_POINT);
    /// 队列状态
    if (ring_queue_Struct->store_state != STORE_EMPTY) {
        ring_queue_Struct->store_state = STORE_NORMAL;
    }
    /// 数据拷贝到外部缓存
    memcpy(pData, ReadBuf, len);
    *data_size_byte = len;

#if STATISTICAL_INFORMATION == 1
    /// 更新统计信息
    ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
    return 0;
}

/**
* @brief                            更新队列统计信息
*
* @param[in]  *pRingBufStruct       帧管理结构体指针
*
*/
static void ring_queue_statistical_information_updata(ring_queue_Type_Def* ring_queue_Struct)
{
#if STATISTICAL_INFORMATION == 1
    
    uint32_t len = 0;
    if (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN) {
        /***
        查找帧头
        **/

        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// 读指针移动
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// 再次重合-无帧头
                ring_queue_Struct->store_state = STORE_EMPTY;
                len = 0;
                break;
            }
            len++;
        }
        ring_queue_Struct->store_free_space = len;
    }
    if (ring_queue_Struct->store_state == STORE_NORMAL) {
        /**
        计算队列剩余字节空间（不代表最终可用空间）
        ***/
        if (ring_queue_Struct->store_w >= ring_queue_Struct->store_r) {             ///< 写 大于 读
            ring_queue_Struct->store_free_space = ring_queue_Struct->store_size - ((uint32_t)ring_queue_Struct->store_w - (uint32_t)ring_queue_Struct->store_r);
        }
        else if (ring_queue_Struct->store_w < ring_queue_Struct->store_r) {  		///< 读 大于 写
            ring_queue_Struct->store_free_space = (uint32_t)ring_queue_Struct->store_r - (uint32_t)ring_queue_Struct->store_w;
        }
        else {

        }
    }
    else if (ring_queue_Struct->store_state == STORE_EMPTY) {
        ring_queue_Struct->store_free_space = ring_queue_Struct->store_size;
    }
    else if (ring_queue_Struct->store_state == STORE_FULL) {
        ring_queue_Struct->store_free_space = 0;
    }
    else {
        // ERROR
    }
    /**
    计算队列空间占用率
    ***/
    ring_queue_Struct->space_occupancy = (ring_queue_Struct->store_size - ring_queue_Struct->store_free_space) * 100 / ring_queue_Struct->store_size;
    if (ring_queue_Struct->space_occupancy_max < ring_queue_Struct->space_occupancy) {
        ring_queue_Struct->space_occupancy_max = ring_queue_Struct->space_occupancy;
    }
#endif STATISTICAL_INFORMATION
}
