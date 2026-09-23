
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_gpio.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       LED翻转（主循环调试用）
                    正常LED节奏由bsp_tim.c的TIM2中断管理

 Function List:     blink()  LED翻转一次
 History:
*******************************************************/

#include "bsp_gpio.h"

void blink(void)
{
    LED1_Toggle();
}
