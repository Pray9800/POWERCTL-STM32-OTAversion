
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_adc.h
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       ADC1(PC1/通道11)动力电压检测采样

 Function List:     Get_Adc()  获取一次ADC原始值
 History:
*******************************************************/

#ifndef __BSP_ADC_H_
#define __BSP_ADC_H_

#include "stm32f1xx_hal.h"
#include "adc.h"

uint16_t Get_Adc(void);

#endif /* __BSP_ADC_H_ */
