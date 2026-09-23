
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_adc.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       ADC1软件触发单次转换，轮询等待EOC
                    （等效IAR原版ADC_GetFlagStatus死等行为）

 Function List:     Get_Adc()  获取一次ADC原始值（12bit）
 History:
*******************************************************/

#include "bsp_adc.h"

// 获取一次ADC原始值（12bit）
// 调用环境含TIM2中断（优先级0高于SysTick），超时参数用HAL_MAX_DELAY
// 避免依赖HAL_GetTick（SysTick在TIM2 ISR内不更新）
uint16_t Get_Adc(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return (uint16_t)HAL_ADC_GetValue(&hadc1);
}
