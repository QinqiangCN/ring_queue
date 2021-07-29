/**
* @file       ring_queue.c
* @brief       ���ζ���
* @details     �ɲ��벻�������ݵĻ��ζ���
* @author      Qinqiang
* @date        2021-7-28
* @version     V1.0
* @copyright   Copyright (c) 2021
**********************************************************************************
* @attention
*
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/07/28  <td>1.0      <td>Qinqiang  <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/

#include <stdint.h>
#include <string.h>

#include "ring_queue.h"

static void ring_queue_statistical_information_updata(ring_queue_Type_Def* ring_queue_Struct);

/**
* @brief                            ָ�����
*
* @param[in]  ring_queue_Struct     ���й���ṹ��ָ��
* @param[in]  point                 ����ָ��ö��
*   WRITE_POINT                     дָ�����
*   READ_POINT                      ��ָ�����
*   TEMP_POINT                      ��ʱָ�����
*
*/
static void add_point(ring_queue_Type_Def* ring_queue_Struct, Enum_Write_or_Read_t point)
{
    if (point == WRITE_POINT) {
        if (++ring_queue_Struct->store_w >= ring_queue_Struct->store_size) {
            ring_queue_Struct->store_w = 0;
        }
        /// д��ʱ��дָ���غϣ��򻺴���
        if (ring_queue_Struct->store_w == ring_queue_Struct->store_r) {
            ring_queue_Struct->store_state = STORE_FULL;
        }
    }
    else if (point == READ_POINT) {
        if (++ring_queue_Struct->store_r >= ring_queue_Struct->store_size) {
            ring_queue_Struct->store_r = 0;
        }
        /// ��ȡʱ��дָ���غϣ��򻺴��
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
* @brief                            ��ʼ������
*
* @param[in]  *ring_queue_Struct    ֡����ṹ��ָ��
* @param[in]  *pBuffer              Ϊ���з���ռ�ָ��
* @param[in]  buffer_size_byte      Ϊ���з���ռ䳤��(byte)
*
* @return                           ����ִ�н��
*   0                               �ɹ�
*   -1                              ʧ��
* @par ʾ��:
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
* @brief                            �����д��һ���ֽ�
*
* @param[in]  *ring_queue_Struct    ���й�����
* @param[in]  byte                  ��д������
*
* @return                           ����ִ�н��
*   0                               �ɹ�
*/
static int ring_queue_write_byte(ring_queue_Type_Def* ring_queue_Struct, uint8_t byte)
{
    /***
    �����ݱ����Ǳ�־
    **/
    if (ring_queue_Struct->store_state == STORE_FULL) {
        ring_queue_Struct->store_state = STORE_FULL_DATA_OVERWRITTEN;
    }
    /// �ֽ�д��
    ((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_w] = byte;
    /// дָ���ƶ�
    add_point(ring_queue_Struct, WRITE_POINT);

    return 0;
}

/**
* @brief                            ���ж�ȡһ���ֽ�
*
* @param[in]  *ring_queue_Struct    ���й�����
*
* @return                           ��ȡ����
*/
static uint8_t ring_queue_read_byte(ring_queue_Type_Def* ring_queue_Struct)
{
    uint8_t byte = 0xFF;

    /// �ֽڶ�ȡ
    byte = ((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r];
    /// ��ָ���ƶ�
    add_point(ring_queue_Struct, READ_POINT);

    return byte;
}

/**
* @brief                            �����д��һ֡����
*
* @param[in]  *ring_queue_Struct    ֡����ṹ��ָ��
* @param[in]  *pData                ��д������ָ��
* @param[in]  data_size_byte        ��д�����ݳ��ȣ�byte��
*
* @return                           ����ִ�н��
*   0                               �ɹ�
*   -1                              ʧ��
*/
int ring_queue_write_frame(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pData, uint32_t data_size_byte)
{
    uint8_t data = 0;

    /***
    �쳣����
    **/
    /// ��ָ���ж�
    if ( (ring_queue_Struct == NULL) || (pData == NULL) ) {
        return -1;
    }
    /// ���ȴ����ж�
    if ( (data_size_byte > ring_queue_Struct->store_size + 2) || (data_size_byte > UINT32_MAX) || (data_size_byte <= 0) ) {
        return -1;
    }
    /***
    Slip Code
    **/
    /// ֡ͷ
    ring_queue_write_byte(ring_queue_Struct, SLIP_START);
    /// ����ÿ���ֽ�
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
    /// ֡β
    ring_queue_write_byte(ring_queue_Struct, SLIP_END);
    /// ֡״̬
    if (ring_queue_Struct->store_state != STORE_FULL_DATA_OVERWRITTEN) {
        ring_queue_Struct->store_state = STORE_NORMAL;
    }
    /// �Ƿ��ڶ��о����ݱ����Ǻ�������ʼѰ���µ�֡ͷ��0�������ã�1�����á�
    ///������ͳ����Ϣʱ��Ҫ���øù��ܣ�
    ///�������øù�����֡���ݳ���С�ڶ��г��ȣ���ʵ��д�볤�ȳ������г���ʱ��Ȼ����д��ɹ���
    if ( ( ( 0 ) && (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN)) || (STATISTICAL_INFORMATION == 1) ) {
        /// ��ָ���ƶ���дָ��λ��
        ring_queue_Struct->store_r = ring_queue_Struct->store_w;
        /***
         ����֡ͷ
         **/
        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// ��ָ���ƶ�
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// �ٴ��غ�-��֡ͷ
                ring_queue_Struct->store_state = STORE_EMPTY;
#if STATISTICAL_INFORMATION == 1
                /// ����ͳ����Ϣ
                ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
                return -1;
            }
        }
        ring_queue_Struct->store_state = STORE_NORMAL;
    }

#if STATISTICAL_INFORMATION == 1
    /// ����ͳ����Ϣ
    ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
    return 0;
}

/**
* @brief                            �Ӷ��ж�ȡһ֡����
*
* @param[in]  *ring_queue_Struct    ֡����ṹ��ָ��
* @param[out] *pData                ��ȡ���ݱ�д��õ�ַ
* @param[out] data_size_byte        ��ȡ֡���ȱ�д��õ�ַ
*
* @return                           ����ִ�н��
*   0                               �ɹ�
*   -1                              ʧ��
*/
int ring_queue_read_frame(ring_queue_Type_Def* ring_queue_Struct, _OUT_ uint8_t* pData, _OUT_ uint32_t *data_size_byte)
{
    uint8_t data = 0;
    uint8_t ReadBuf[520] = { 0 };
    uint32_t buf_count = 0;
    uint32_t len = 0;

    /***
    �쳣����
    **/
    /// ��ָ���ж�
    if (ring_queue_Struct == NULL || pData == NULL) {
        return -1;
    }
    ///// ���ȳ����ж�
    //if (data_size_byte > UINT32_MAX) {
    //    return -1;
    //}
    /// ���п��ж�
    if (ring_queue_Struct->store_state == STORE_EMPTY) {
        return -1;
    }

    if (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN) {
        /// ��ָ���ƶ���дָ��λ��
        ring_queue_Struct->store_r = ring_queue_Struct->store_w;
        /***
        ����֡ͷ
        **/
        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// ��ָ���ƶ�
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// �ٴ��غ�-��֡ͷ
                ring_queue_Struct->store_state = STORE_EMPTY;
                return -1;
            }
        }
    }
    /// ��ָ���ƶ�-�Թ�֡ͷ
    add_point(ring_queue_Struct, READ_POINT);

    do {
        /***
        Slip Code
        **/
        /// ����һ���ֽ�
        data = ring_queue_read_byte(ring_queue_Struct);
        if (data == SLIP_ESC) {
            ReadBuf[buf_count++] = ring_queue_read_byte(ring_queue_Struct) - 1;
        }
        else {
            ReadBuf[buf_count++] = data;
        }
        /// ���ȼ���
        len++;
        
        if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
            /// �ٴ��غ�-��֡β
            ring_queue_Struct->store_state = STORE_EMPTY;
            return -1;
        }
    } while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_END);
    /// ��ָ���ƶ�-�Թ�֡β
    add_point(ring_queue_Struct, READ_POINT);
    /// ����״̬
    if (ring_queue_Struct->store_state != STORE_EMPTY) {
        ring_queue_Struct->store_state = STORE_NORMAL;
    }
    /// ���ݿ������ⲿ����
    memcpy(pData, ReadBuf, len);
    *data_size_byte = len;

#if STATISTICAL_INFORMATION == 1
    /// ����ͳ����Ϣ
    ring_queue_statistical_information_updata(ring_queue_Struct);
#endif
    return 0;
}

/**
* @brief                            ���¶���ͳ����Ϣ
*
* @param[in]  *pRingBufStruct       ֡����ṹ��ָ��
*
*/
static void ring_queue_statistical_information_updata(ring_queue_Type_Def* ring_queue_Struct)
{
#if STATISTICAL_INFORMATION == 1
    
    uint32_t len = 0;
    if (ring_queue_Struct->store_state == STORE_FULL_DATA_OVERWRITTEN) {
        /***
        ����֡ͷ
        **/

        while (((uint8_t*)(ring_queue_Struct->pBuf))[ring_queue_Struct->store_r] != SLIP_START) {
            /// ��ָ���ƶ�
            add_point(ring_queue_Struct, READ_POINT);

            if (ring_queue_Struct->store_r == ring_queue_Struct->store_w) {
                /// �ٴ��غ�-��֡ͷ
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
        �������ʣ���ֽڿռ䣨���������տ��ÿռ䣩
        ***/
        if (ring_queue_Struct->store_w >= ring_queue_Struct->store_r) {             ///< д ���� ��
            ring_queue_Struct->store_free_space = ring_queue_Struct->store_size - ((uint32_t)ring_queue_Struct->store_w - (uint32_t)ring_queue_Struct->store_r);
        }
        else if (ring_queue_Struct->store_w < ring_queue_Struct->store_r) {  		///< �� ���� д
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
    ������пռ�ռ����
    ***/
    ring_queue_Struct->space_occupancy = (ring_queue_Struct->store_size - ring_queue_Struct->store_free_space) * 100 / ring_queue_Struct->store_size;
    if (ring_queue_Struct->space_occupancy_max < ring_queue_Struct->space_occupancy) {
        ring_queue_Struct->space_occupancy_max = ring_queue_Struct->space_occupancy;
    }
#endif STATISTICAL_INFORMATION
}
