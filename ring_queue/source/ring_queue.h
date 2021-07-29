#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#define _OUT_
#define _IN_

#define STATISTICAL_INFORMATION     1                                    ///< 统计信息 1：启用，0：不启用

/// RTOS Select
#define OS_FREERTOS                 1
#define OS_RT_THREAD                2
#define OS_THREADX                  3

#define OS_NONE                     4
#define OS_NOT_USE                  5

#define OS_USED                     OS_NONE

#if     OS_USED == OS_FREERTOS
/***
FREERTOS Mutex
**/
#include "semphr.h"
#define OSIF_MUTEX_VARIABLE         ring_queue_Struct->xMutex
#define OSIF_MUTEX_DECLARE          SemaphoreHandle_t xMutex
#define OSIF_MUTEX_CREATE()         OSIF_MUTEX_VARIABLE = xSemaphoreCreateMutex()
#define OSIF_MUTEX_WAIT()           xSemaphoreTake( OSIF_MUTEX_VARIABLE, portMAX_DELAY )
#define OSIF_MUTEX_RELEASE()        xSemaphoreGive( OSIF_MUTEX_VARIABLE )
#elif   OS_USED == OS_RT_THREAD
/***
RT-Thread
**/
#define OSIF_MUTEX_VALUE()          
#define OSIF_MUTEX_CREATE()         
#define OSIF_MUTEX_WAIT()           
#define OSIF_MUTEX_RELEASE()        
#elif   OS_USED == OS_NONE
/***
NO os
**/
#define OSIF_MUTEX_VARIABLE         ring_queue_Struct->xMutex
#define OSIF_MUTEX_DECLARE          uint8_t xMutex
#define OSIF_MUTEX_CREATE()         OSIF_MUTEX_VARIABLE = 0
#define OSIF_MUTEX_WAIT()           while(OSIF_MUTEX_VARIABLE); OSIF_MUTEX_VARIABLE = 1
#define OSIF_MUTEX_RELEASE()        OSIF_MUTEX_VARIABLE = 0
#elif   OS_USED == OS_NOT_USE
/***
Undefined Mutex
**/
#define OSIF_MUTEX_VALUE()         
#define OSIF_MUTEX_CREATE()       
#define OSIF_MUTEX_WAIT()         
#define OSIF_MUTEX_RELEASE()
#else
#error
#endif

/// 队列状态枚举
typedef enum {
    STORE_EMPTY = 0,                                                     ///< 队列空
    STORE_NORMAL,                                                        ///< 队列正常
    STORE_FULL,                                                          ///< 队列满
    STORE_FULL_DATA_OVERWRITTEN                                          ///< 队列旧数据被覆盖（队列第一帧数据被破坏）
}Enum_ring_buffer_store_state_t;

/// 指针操作枚举
typedef enum {
    WRITE_POINT,                                                         ///< 写
    READ_POINT,                                                          ///< 读
    TEMP_POINT,                                                          ///< 临时
}Enum_Write_or_Read_t;

/// 写操作 新帧|追加 操作枚举
typedef enum {
    WRITE_FRAME_NEW,                                                     ///< 写入新的一帧
    WRITE_FRAME_APPEND,                                                  ///< 追加数据
}Enum_Write_mode_t;

/// Slip Code
typedef enum
{
    SLIP_START = 0xAB,
    SLIP_ESC_START,

    SLIP_END = 0xBC,
    SLIP_ESC_END,

    SLIP_ESC = 0xCD,
    SLIP_ESC_ESC,
}Enum_SlipCode_t;

/// <summary>
/// 帧管理结构体枚举
/// </summary>
typedef struct {
    void* pBuf;                                                          ///< 缓存指针

    uint32_t store_size;                                                 ///< 缓存大小
    uint32_t store_w;                                                    ///< 写指针
    uint32_t store_r;                                                    ///< 读指针
    uint32_t store_temp;                                                 ///< 临时指针

    Enum_ring_buffer_store_state_t store_state;                          ///< 缓存状态

    uint32_t store_free_space;                                           ///< 缓存空闲大小
    uint32_t space_occupancy;                                            ///< 空间占用率
    uint32_t space_occupancy_max;                                        ///< 最大空间占用率

    OSIF_MUTEX_DECLARE;                                                  ///< 互斥信号量
}ring_queue_Type_Def;



int ring_queue_init(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pBuffer, uint32_t buffer_size_byte);

int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_mode_t mode, uint8_t* pData, uint32_t data_size_byte);

int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte);


#endif

