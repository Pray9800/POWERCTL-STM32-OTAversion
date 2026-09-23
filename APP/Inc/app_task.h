
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         app_task.h
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       动力切换主任务对外接口与全局变量

 Function List:     task_Start()  主任务入口（初始化+主循环）
 History:
*******************************************************/

#ifndef __APP_TASK_H_
#define __APP_TASK_H_

#include "stm32f1xx_hal.h"

// OTA升级后用VERSION命令查询应答（UART3）
#define FW_VERSION "FW-JT004-02 V1.1.0.20260923"
#define FW_DATE    "2026-09-23"

// 动力通道选择值（ 01=15V档、02=12V档）
#define CTL_SWITCH_NONE  0  // 无动力（电压>阈值4000，断开）
#define CTL_SWITCH_15V   1  // 15V档（PB1=CTL_15继电器=15V输出）
#define CTL_SWITCH_12V   2  // 12V档（PB0=CTL_12继电器=12V输出）

extern uint8_t CTL_SWITCH;      // 当前动力档位：0=断开 1=15V档 2=12V档，TIM2中断按ADC阈值刷新
extern uint8_t CTL_SWITCH_EN;   // 输出使能：0=双断 非0=允许输出，上位机0x0024写指令data[1]赋值
extern uint8_t CTL_SWITCH_FLAG;  // 继电器刷新触发标志：收到0x0024读/写帧置1，主循环执行一次relay_switch后清零
extern uint8_t CTL_SWITCH_prv; // 上次上报的档位记录，和当前档位不同就主动发状态帧

void task_Start(void);

#endif /* __APP_TASK_H_ */
