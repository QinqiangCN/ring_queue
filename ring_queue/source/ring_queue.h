#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#define _OUT_
#define _IN_

#define STATISTICAL_INFORMATION     1                                    ///< ͳ����Ϣ 1�����ã�0��������

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

/// ����״̬ö��
typedef enum {
    STORE_EMPTY = 0,                                                     ///< ���п�
    STORE_NORMAL,                                                        ///< ��������
    STORE_FULL,                                                          ///< ������
    STORE_FULL_DATA_OVERWRITTEN                                          ///< ���о����ݱ����ǣ����е�һ֡���ݱ��ƻ���
}Enum_ring_buffer_store_state_t;

/// ָ�����ö��
typedef enum {
    WRITE_POINT,                                                         ///< д
    READ_POINT,                                                          ///< ��
    TEMP_POINT,                                                          ///< ��ʱ
}Enum_Write_or_Read_t;

/// д���� ��֡|׷�� ����ö��
typedef enum {
    WRITE_FRAME_NEW,                                                     ///< д���µ�һ֡
    WRITE_FRAME_APPEND,                                                  ///< ׷������
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
/// ֡����ṹ��ö��
/// </summary>
typedef struct {
    void* pBuf;                                                          ///< ����ָ��

    uint32_t store_size;                                                 ///< �����С
    uint32_t store_w;                                                    ///< дָ��
    uint32_t store_r;                                                    ///< ��ָ��
    uint32_t store_temp;                                                 ///< ��ʱָ��

    Enum_ring_buffer_store_state_t store_state;                          ///< ����״̬

    uint32_t store_free_space;                                           ///< ������д�С
    uint32_t space_occupancy;                                            ///< �ռ�ռ����
    uint32_t space_occupancy_max;                                        ///< ���ռ�ռ����

    OSIF_MUTEX_DECLARE;                                                  ///< �����ź���
}ring_queue_Type_Def;



int ring_queue_init(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pBuffer, uint32_t buffer_size_byte);

int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_mode_t mode, uint8_t* pData, uint32_t data_size_byte);

int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte);


#endif

