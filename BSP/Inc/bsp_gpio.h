
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_gpio.h
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       应用层GPIO操作宏与LED控制

                    继电器控制：PB0=CTL_12=12V输出 PB1=CTL_15=15V输出
                    （IAR bsp_uart.c:118注释把01/02档位标反，实际01=15V档、02=12V档）

 Function List:     blink()  LED翻转一次
 History:
*******************************************************/

#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

#include "stm32f1xx_hal.h"
#include "main.h"

#define CTL_12_ON()    HAL_GPIO_WritePin(CTL_12_GPIO_Port, CTL_12_Pin, GPIO_PIN_SET)        //12V PB0
#define CTL_15_ON()    HAL_GPIO_WritePin(CTL_15_GPIO_Port, CTL_15_Pin, GPIO_PIN_SET)        //15V PB1
#define CTL_12_OFF()   HAL_GPIO_WritePin(CTL_12_GPIO_Port, CTL_12_Pin, GPIO_PIN_RESET)   //12V PB0
#define CTL_15_OFF()   HAL_GPIO_WritePin(CTL_15_GPIO_Port, CTL_15_Pin, GPIO_PIN_RESET)   //15V PB1

// PB3 = LED1，需JTAG已禁用（HAL_MspInit中已执行__HAL_AFIO_REMAP_SWJ_NOJTAG）
#define LED1_Toggle()  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)
#define LED1_OFF()     HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)

void blink(void);

#endif /* __BSP_GPIO_H_ */
