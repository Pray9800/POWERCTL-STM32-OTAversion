
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_usart.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       动力切换板上位机通讯（USART3）
                    本文件只做帧接收 + CRC校验 + 置标志，
                    命令执行和应答都在 app_task.c 里处理

 Function List:     MCU_HOST_Send()           阻塞发送
                    Go_To_Bootloader()        写OTA暗号后软复位进Bootloader
                    HAL_UART_RxCpltCallback() 单字节中断接收链
 History:
*******************************************************/

#include "bsp_usart.h"
#include "bsp_crc.h"
#include <string.h>

#define UPDATE_FIRMWARE_CMD_LEN 15
#define VERSION_CMD_LEN         7

uint8_t rx_temp_u3 = 0;   // UART3字节中断接收缓存
uint8_t UART3_Rxbuff[8] = {0};
volatile uint8_t HOST_Cmd_ready = 0;   // 接收完成标志 命令功能(0x06写)
volatile uint8_t HOST_Read_ready = 0;  // 接收完成标志 查询功能(0x03读)

// 阻塞发送（无DMA），主循环和中断回调均可调用
void MCU_HOST_Send(uint8_t *buf, uint8_t len)
{
    HAL_UART_Transmit(&huart3, buf, len, 100);
}

// 复位进Bootloader：软复位会禁用软件模式IWDG，升级期间无看门狗复位风险
void Go_To_Bootloader(void)
{
    /* 1. 使能 PWR(电源) 和 BKP(后备区域) 的时钟 */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    /* 2. 取消后备区域的写保护 */
    HAL_PWR_EnableBkUpAccess();

    /* 3. 直接操作底层寄存器写入暗号 */
    BKP->DR1 = 0x55AA;

    /* 4. 软件强制重启 */
    NVIC_SystemReset();
}

// UART3接收完成回调（单字节中断链）
// 1. 滑动窗口匹配ASCII暗号：UPDATE_FIRMWARE(升级) / VERSION(查版本)
// 2. MODBUS帧状态机：0x5B帧头 + 0x06/0x03功能码，收满8字节做CRC16校验，通过后置标志
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static char ota_buf[UPDATE_FIRMWARE_CMD_LEN + 1] = {0}; // 15个字母 + 1个结束符
    static char ver_buf[VERSION_CMD_LEN + 1] = {0};         // 7个字母 + 1个结束符
    static uint8_t rx_cnt_u3 = 0;  // UART3计数器，记录当前拼接到第几个字节了

    if (huart->Instance == USART3)  // 来自上位机
    {
        // ==========================================================
        //   OTA 暗号滑动窗口拦截 (独立于下方的二进制状态机)
        // ==========================================================

        // 整体往前挪一个位置，把最老的字符挤掉
        memmove(ota_buf, ota_buf + 1, UPDATE_FIRMWARE_CMD_LEN - 1);
        // 把新收到的字符塞到最后
        ota_buf[UPDATE_FIRMWARE_CMD_LEN - 1] = (char)rx_temp_u3;

        // 如果这连续的15个字符刚好凑成了 "UPDATE_FIRMWARE"
        if (strcmp(ota_buf, "UPDATE_FIRMWARE") == 0)
        {
            Go_To_Bootloader(); // 瞬间自杀并跳转，后面的代码都不管了！
        }

        // VERSION 查询指令
        memmove(ver_buf, ver_buf + 1, VERSION_CMD_LEN - 1);
        ver_buf[VERSION_CMD_LEN - 1] = (char)rx_temp_u3;
        if (strcmp(ver_buf, "VERSION") == 0)
        {
            HAL_UART_Transmit(&huart3, (uint8_t *)FW_VERSION, strlen(FW_VERSION), 1000);
            HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 1000);
        }

        // ==========================================================
        //   MODBUS帧状态机 (5B 06/03 ... 8字节)
        // ==========================================================
        if (rx_cnt_u3 == 0)  // 还没收到数据，先找帧头5B
        {
            if (rx_temp_u3 == 0x5B)
            {
                UART3_Rxbuff[0] = rx_temp_u3;
                rx_cnt_u3 = 1;
            }
        }
        else if (rx_cnt_u3 == 1)  // 帧头已符合，第二个字节必须是功能码06或03
        {
            if (rx_temp_u3 == 0x06 || rx_temp_u3 == 0x03)
            {
                UART3_Rxbuff[1] = rx_temp_u3;
                rx_cnt_u3 = 2;
            }
            else  // 5B 后面跟的不是 06/03，直接复位重找
            {
                rx_cnt_u3 = 0;
                if (rx_temp_u3 == 0x5B) // 连续帧头，重新同步
                {
                    UART3_Rxbuff[0] = rx_temp_u3;
                    rx_cnt_u3 = 1;
                }
            }
        }
        else  // 包头 5B + 功能码 确认无误，开始接收后续数据
        {
            UART3_Rxbuff[rx_cnt_u3] = rx_temp_u3;
            rx_cnt_u3++;

            if (rx_cnt_u3 >= 8)  // 收满8个字节，做CRC校验
            {
                uint16_t calculated_crc = CRC16_Modbus(UART3_Rxbuff, 6);  // 校验前6字节
                uint16_t received_crc = (uint16_t)((UART3_Rxbuff[7] << 8) | UART3_Rxbuff[6]);
                if (calculated_crc == received_crc)
                {
                    if (UART3_Rxbuff[1] == 0x06)  // 写指令06
                    {
                        HOST_Cmd_ready = 1;   // 置标志，主循环处理写指令
                    }
                    else  // 读取指令03
                    {
                        HOST_Read_ready = 1;  // 置标志，主循环处理查询指令
                    }
                }
                rx_cnt_u3 = 0;
            }
        }

        //再次开启单字节收取
        HAL_UART_Receive_IT(&huart3, &rx_temp_u3, 1);
    }
}
