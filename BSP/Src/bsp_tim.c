
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_tim.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       TIM2定时器（0.5ms中断）
                    移植自IAR原版bsp_timer.c的TIM2中断（原版arr=1000,psc=35,0.5ms）：
                      - LED节奏（25ms节拍）：无使能=100ms快闪；15V档=250ms翻一次；12V档=1s翻一次
                      - ADC采样（2.5ms周期）：PC1一阶IIR滤波（15/20旧值+5/20新值）
                      - 阈值判断：滤波值>4000→0(断) >2000→1(15V档) >900→2(12V档)

                    帧命令执行不在中断中（本工程约定：bsp_usart置标志，app_task主循环执行）

 Function List:     Sys_Delay()                      毫秒延时
                    TIM2_IT_Start()                  启动TIM2中断
                    HAL_TIM_PeriodElapsedCallback()  定时中断回调
 History:
*******************************************************/

#include "bsp_tim.h"
#include "bsp_gpio.h"
#include "bsp_adc.h"

extern uint8_t CTL_SWITCH;    // 当前动力档位，app_task.c定义
extern uint8_t CTL_SWITCH_EN; // 动力输出使能，app_task.c定义

static uint16_t adc_voltage = 0; // PC1滤波后的ADC值

void Sys_Delay(uint16_t time_ms)
{
    HAL_Delay(time_ms);
}

void TIM2_IT_Start(void)
{
    HAL_TIM_Base_Start_IT(&htim2);
}

// TIM2溢出中断（2khz），LED节奏指示 + ADC采样判档
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t  time_3state = 0;
    static uint8_t timer_cnt = 0;
    static uint8_t led_cnt[3] = {0, 0, 0};

    if (htim->Instance == TIM2)
    {
        /* LED节奏：25ms节拍 */
         time_3state++;
        if ( time_3state >= LED_TICK_CNT)
        {
             time_3state = 0;
            if (CTL_SWITCH_EN)
            {
                if (CTL_SWITCH == 2)        /* 12V档：1s翻转 */
                {
                    led_cnt[0]++;
                    if (led_cnt[0] >= 8) /* 8 x 25ms = 1s */
                    {
                        led_cnt[0] = 0;
                        LED1_Toggle();
                    }
                }
                else if (CTL_SWITCH == 1)   /* 15V档：250ms翻转 */
                {
                    led_cnt[1]++;
                    if (led_cnt[1] >= 4) /* 4 x 25ms = 100ms */
                    {
                        led_cnt[1] = 0;
                        LED1_Toggle();
                    }
                }
            }
            else
            {
                led_cnt[2]++;
                if (led_cnt[2] >= 40) /* 40 x 25ms = 1000ms翻转：未使能 */
                {
                    led_cnt[2] = 0;
                    LED1_Toggle();
                }
            }
        }

        /* ADC采样与阈值判断：5ms周期 */
     // 标志位 等待主循环去执行通断电
        timer_cnt++;
        if (timer_cnt >= ADC_SCHEDULE_CNT)
        {
            timer_cnt = 0;
            uint16_t adc_tmp = Get_Adc();
            adc_voltage = adc_voltage * 15 / 20 + adc_tmp * 5 / 20; //一阶IIR滤波
            if (adc_voltage > 4000)
            {
                CTL_SWITCH = 0;  //过压，断开
            }
            else if (adc_voltage > 2000)
            {
                CTL_SWITCH = 1;  //15V档
            }
            else if (adc_voltage > 900)
            {
                CTL_SWITCH = 2;  //12V档
            }
        }
    }
}
