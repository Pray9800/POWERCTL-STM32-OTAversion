
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         app_task.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       动力切换主任务（移植自IAR工程JT_101_stm32f103_ctrlboard_1/main.c）

                    上位机协议（USART3/PB10/PB11，115200，8字节MODBUS帧）：
                      寄存器0x0009 心跳：回显原帧
                      寄存器0x0024 电源选择：
                        读(0x03)：回复 5B 03 00 24 00 <CTL_SWITCH> crcL crcH
                        写(0x06)：data[1]=使能(非0=输出开)，回显原帧，触发继电器状态机
                      通道变化主动上报：5B 03 00 24 00 <CTL_SWITCH> crcL crcH

                    OTA：UART3收到ASCII "UPDATE_FIRMWARE" → 写BKP DR1=0x55AA → 软复位进Bootloader

 Function List:     task_init()    上电初始化（延时/看门狗/TIM2/继电器全断）
                    power_switch() 继电器切换状态机（收到0x0024帧触发）
                    task_Start()   主任务入口（初始化+主循环）
 History:
*******************************************************/

#include "app_task.h"
#include "bsp_usart.h"
#include "bsp_gpio.h"
#include "bsp_tim.h"
#include "bsp_iwdg.h"
#include "bsp_crc.h"
#include <string.h>

uint8_t CTL_SWITCH = 0;      // 当前动力档位：0=断开 1=15V档 2=12V档
uint8_t CTL_SWITCH_EN = 0;   // 输出使能：0=双断 非0=允许输出
uint8_t CTL_SWITCH_FLAG = 0;  // 收到0x0024帧标志，触发继电器状态机
uint8_t CTL_SWITCH_prv = 0;  // 上次上报的档位，用于变化时主动上报

#define MODBUS_REG_HEARTBEAT  0x0009
#define MODBUS_REG_PWRSELECT  0x0024

static void task_init(void)
{
    Sys_Delay(3000); //初始化三秒延时
    IWDG_ON(); //开启看门狗
    TIM2_IT_Start(); //开启TIM2中断，0.5ms节拍

    /* 上电继电器断开 */
    CTL_12_OFF(); // 关闭12v继电器
    CTL_15_OFF();  //关闭15v 继电器

    HAL_UART_Receive_IT(&huart3, &rx_temp_u3, 1); //开启U3接收中断
}

// 继电器切换状态机（收到0x0024帧触发）
// 切换时序：先断旧通道继电器 → 20ms间隔（等旧触点释放）→ 合新通道继电器
static void power_switch(void)
{
    if (CTL_SWITCH_EN)
    {
        if (CTL_SWITCH == CTL_SWITCH_15V)      /* 15V档：断12V路(PB0)，合15V路(PB1) */
        {
            CTL_12_OFF();
            Sys_Delay(20);
            CTL_12_OFF();
            CTL_15_ON();
        }
        else if (CTL_SWITCH == CTL_SWITCH_12V) /* 12V档：断15V路(PB1)，合12V路(PB0) */
        {
            CTL_15_OFF();
            Sys_Delay(20);
            CTL_12_ON();
            CTL_15_OFF();
        }
    }
    else
    {
        CTL_12_OFF();
        CTL_15_OFF();
    }
}

void task_Start(void)
{
    uint8_t temp_tx_buf[8];

    task_init();

    while (1)
    {
        Sys_Delay(10);

        /* 上位机 写命令帧（0x06） */
        if (HOST_Cmd_ready)
        {
            HOST_Cmd_ready = 0; //清除标志位
            memcpy(temp_tx_buf, UART3_Rxbuff, 8); //复制
            uint16_t reg_addr = (uint16_t)((temp_tx_buf[2] << 8) | temp_tx_buf[3]); //命令读取

            if (reg_addr == MODBUS_REG_PWRSELECT)  //上位机使能控制
            {
                CTL_SWITCH_EN = temp_tx_buf[5];  // data[1]=使能
                CTL_SWITCH_FLAG = 1;              // 触发继电器状态机
                MCU_HOST_Send(temp_tx_buf, 8);   // 回chuasnhu 原帧
            }
            else if (reg_addr == MODBUS_REG_HEARTBEAT)
            {
                MCU_HOST_Send(temp_tx_buf, 8);   // 心跳回传输
            }
        }

        /* 读命令帧（0x03） */
        if (HOST_Read_ready) //查询
        {
            HOST_Read_ready = 0;
            memcpy(temp_tx_buf, UART3_Rxbuff, 8);
            uint16_t reg_addr = (uint16_t)((temp_tx_buf[2] << 8) | temp_tx_buf[3]);

            if (reg_addr == MODBUS_REG_PWRSELECT)
            {
                temp_tx_buf[4] = 0x00;
                temp_tx_buf[5] = CTL_SWITCH;     // 回复当前档位
                uint16_t crc = CRC16_Modbus(temp_tx_buf, 6);
                temp_tx_buf[6] = (uint8_t)(crc & 0xFF); //CRC低字节
                temp_tx_buf[7] = (uint8_t)(crc >> 8);  //CRC高字节
                MCU_HOST_Send(temp_tx_buf, 8); //帧头+内容+CRC
                CTL_SWITCH_FLAG = 1;              // 读也触发继电器状态机（与IAR一致）
            }
            else
            {
                MCU_HOST_Send(temp_tx_buf, 8);   // 其他寄存器回显
            }
        }

        /* 通道变化主动上报 */
        if (CTL_SWITCH != CTL_SWITCH_prv) //TIM2中断处读取与历史不一致
        {
            uint8_t sendbuf[8] = {0x5B, 0x03, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00};
            sendbuf[5] = CTL_SWITCH; //保存到数组
            uint16_t crc = CRC16_Modbus(sendbuf, 6);
            sendbuf[6] = (uint8_t)(crc & 0xFF);
            sendbuf[7] = (uint8_t)(crc >> 8);
            MCU_HOST_Send(sendbuf, 8);
            CTL_SWITCH_prv = CTL_SWITCH;
        }

        /* 继电器状态机 */
        if (CTL_SWITCH_FLAG)  //标志位 由上位机控制
        {
            CTL_SWITCH_FLAG = 0;
            power_switch();
        }

        /* 无动力时强制断开 上一层保险 */
        if (CTL_SWITCH == CTL_SWITCH_NONE)  //检测到拔掉
        {
            CTL_SWITCH_EN = 0;
            CTL_12_OFF();
            CTL_15_OFF();
        }

        IWDG_Refresh();
    }
}
