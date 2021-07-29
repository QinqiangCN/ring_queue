#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "ring_queue.h"


#define BUF1_NUM  30
uint8_t buf1[BUF1_NUM];

#define BUF2_NUM  10
uint8_t buf2[BUF2_NUM];

#define BUF3_NUM  23
uint8_t buf3[BUF3_NUM];

#define BUF4_NUM  16
uint8_t buf4[BUF4_NUM];

#define BUF5_NUM  8
uint8_t buf5[BUF5_NUM];

#define BUF6_NUM  18
uint8_t buf6[BUF6_NUM];


ring_queue_Type_Def  uart_rx_queue_handle;
uint8_t              uart_rx_queue_buf[1000];
uint8_t              ReadBuf[520];
uint32_t             Read_len;

/// <summary>
/// 读取一帧数据测试
/// </summary>
/// <param name="ring_queue_Struct"></param>
void read_test(ring_queue_Type_Def* ring_queue_Struct)
{
    static num = 1;
    // 读
    printf("\r\n#<-- 开始第%d次读取\r\n", num);
    
    if (ring_queue_read_frame(ring_queue_Struct, ReadBuf, &Read_len) != 0)
    {
        printf("  读取错误！\n\r");
    }
    else
    {
        printf("  读取长度：%d\n\r", Read_len);

        if (memcmp(buf1, ReadBuf, BUF1_NUM) == 0) {
            printf("  读取完成,数据与buf1相同！。\n\r");
        }
        else if (memcmp(buf2, ReadBuf, BUF2_NUM) == 0) {
            printf("  读取完成,数据与buf2相同！。\n\r");
        }
        else if (memcmp(buf3, ReadBuf, BUF3_NUM) == 0) {
            printf("  读取完成,数据与buf3相同！。\n\r");
        }
        else if (memcmp(buf4, ReadBuf, BUF4_NUM) == 0) {
            printf("  读取完成,数据与buf4相同！。\n\r");
        }
        else if (memcmp(buf5, ReadBuf, BUF5_NUM) == 0) {
            printf("  读取完成,数据与buf5相同！。\n\r");
        }
        else if (memcmp(buf6, ReadBuf, BUF6_NUM) == 0) {
            printf("  读取完成,数据与buf6相同！。\n\r");
        }
        else {
            printf("  读取完成,未找到匹配的bufx！。\n\r");
        }
    }
    num++;
#if STATISTICAL_INFORMATION == 1
    printf("  #    剩余空间%d(B)\n\r", ring_queue_Struct->store_free_space);
    printf("  #    已使用%d%%\n\r", ring_queue_Struct->space_occupancy);
#endif
}

/// <summary>
/// 写入一帧数据测试
/// </summary>
/// <param name="ring_queue_Struct"></param>
/// <param name="pData"></param>
/// <param name="data_size_byte"></param>
/// <param name="tag"></param>
void write_test(ring_queue_Type_Def* ring_queue_Struct, uint8_t* pData, uint32_t data_size_byte, uint32_t tag)
{
    uint8_t flg = 0;

    printf("\r\n#--> buf%d数据写入...", tag);
    ring_queue_Struct->store_temp = ring_queue_Struct->store_w;
    if (ring_queue_write_frame(&uart_rx_queue_handle, WRITE_FRAME_NEW, pData, data_size_byte) != 0) {
        printf("失败！\r\n");
    }
    else {
        printf("成功！数据长度%d(B), \n\r", data_size_byte);
        //if (ring_queue_Struct->store_w >= ring_queue_Struct->store_temp) {             ///< 写 大于 读
        //    data_size_byte = (uint32_t)ring_queue_Struct->store_w - (uint32_t)ring_queue_Struct->store_temp;
        //}
        //else if (ring_queue_Struct->store_w < ring_queue_Struct->store_temp) {  		///< 读 大于 写
        //    data_size_byte = ring_queue_Struct->store_size - ((uint32_t)ring_queue_Struct->store_temp - (uint32_t)ring_queue_Struct->store_w);
        //}
        //printf("写入长度%d(B)\r\n", data_size_byte);
    }
#if STATISTICAL_INFORMATION == 1
    printf("  #    剩余空间%d(B)\n\r", ring_queue_Struct->store_free_space);
    printf("  #    已使用%d%%\n\r", ring_queue_Struct->space_occupancy);
#endif
}

/// <summary>
/// 主函数
/// </summary>
/// <param name=""></param>
/// <returns></returns>
int main(void)
{
    printf("Run.\n\r");

    // init buf
    for (int i = 0; i < BUF1_NUM; i++) {
        buf1[i] = rand();
    }
    for (int i = 0; i < BUF2_NUM; i++) {
        buf2[i] = rand();
    }
    for (int i = 0; i < BUF3_NUM; i++) {
        buf3[i] = rand();
    }
    for (int i = 0; i < BUF4_NUM; i++) {
        buf4[i] = rand();
    }
    for (int i = 0; i < BUF5_NUM; i++) {
        buf5[i] = rand();
    }
    for (int i = 0; i < BUF6_NUM; i++) {
        buf6[i] = rand();
    }

    // 测试-插入特殊转义变量
    buf1[3] = 0xAB;
    buf1[8] = 0xBC;
    buf2[5] = 0xCD;


    /// 队列初始化
    if (ring_queue_init(&uart_rx_queue_handle, uart_rx_queue_buf, sizeof(uart_rx_queue_buf)) != 0) {
        printf("初始化失败\r\n");
    }
    else {
        printf("#  初始化成功\r\n");
        printf("#  队列大小：%d（B）\r\n", uart_rx_queue_handle.store_size);
    }

    /***
    写入、读取测试
    **/
    write_test(&uart_rx_queue_handle, &buf1, BUF1_NUM, 1);
    read_test(&uart_rx_queue_handle);
    write_test(&uart_rx_queue_handle, &buf2, BUF2_NUM, 2);
    read_test(&uart_rx_queue_handle);
    write_test(&uart_rx_queue_handle, &buf3, BUF3_NUM, 3);
    read_test(&uart_rx_queue_handle);
    write_test(&uart_rx_queue_handle, &buf4, BUF4_NUM, 4);
    write_test(&uart_rx_queue_handle, &buf5, BUF5_NUM, 5);
    write_test(&uart_rx_queue_handle, &buf6, BUF6_NUM, 6);
    read_test(&uart_rx_queue_handle);
    read_test(&uart_rx_queue_handle);
    read_test(&uart_rx_queue_handle);

    read_test(&uart_rx_queue_handle);

    printf("\r\n#--> buf2数据写入...");
    if (ring_queue_write_frame(&uart_rx_queue_handle, WRITE_FRAME_NEW, buf2, BUF2_NUM) != 0) {
        printf("失败！\r\n");
    }
    else {
        printf("成功！\r\n");
    }

    printf("\r\n#--> buf2数据追加写入...");
    if (ring_queue_write_frame(&uart_rx_queue_handle, WRITE_FRAME_APPEND, buf2, BUF2_NUM) != 0) {
        printf("失败！\r\n");
    }
    else {
        printf("成功！\r\n");
    }


#if STATISTICAL_INFORMATION == 1
    printf("\n\r# 最大占用率%d%%", uart_rx_queue_handle.space_occupancy_max);
#endif

    return 0;
}

