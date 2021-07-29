#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#define _OUT_
#define _IN_

#define STATISTICAL_INFORMATION  1                                       ///< 统计信息 1：启用，0：不启用

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

    //rt_mutex_t muctexlock_storewr;                                     ///< 互斥锁
}ring_queue_Type_Def;



int ring_queue_init(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pBuffer, uint32_t buffer_size_byte);

int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_mode_t mode, uint8_t* pData, uint32_t data_size_byte);

int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte);


#endif

