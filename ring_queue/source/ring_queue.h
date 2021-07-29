#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#define _OUT_
#define _IN_

#define STATISTICAL_INFORMATION  1                                       ///< ͳ����Ϣ 1�����ã�0��������

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

    //rt_mutex_t muctexlock_storewr;                                     ///< ������
}ring_queue_Type_Def;



int ring_queue_init(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pBuffer, uint32_t buffer_size_byte);

int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_mode_t mode, uint8_t* pData, uint32_t data_size_byte);

int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte);


#endif

