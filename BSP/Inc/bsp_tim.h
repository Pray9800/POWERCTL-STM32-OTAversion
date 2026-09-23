
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_tim.h
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       TIM2定时中断（0.5ms）与毫秒延时

 Function List:     Sys_Delay()      毫秒延时
                    TIM2_IT_Start()  启动TIM2中断
 History:
*******************************************************/

#ifndef __BSP_TIM_H_
#define __BSP_TIM_H_

#include "stm32f1xx_hal.h"
#include "tim.h"

// TIM2中断周期：72MHz/36/1000 = 0.5ms（与IAR原版bsp_timer_Init(1000,36-1)一致）
#define TIM2_IT_PERIOD_US   500u

// ADC调度周期：10 x 0.5ms = 5ms（5ms采样一次）
#define ADC_SCHEDULE_CNT    10u 

// LED节奏节拍：50 x 0.5ms = 25ms（与IAR原版time_3S>=50一致）
#define LED_TICK_CNT        50u

void Sys_Delay(uint16_t time_ms);
void TIM2_IT_Start(void);

#endif /* __BSP_TIM_H_ */
