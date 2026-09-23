
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_usart.h
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       USART3通讯：OTA升级暗号识别、版本查询、
                    上位机MODBUS帧接收、应答发送

 Function List:     MCU_HOST_Send()    阻塞发送
                    Go_To_Bootloader() 写OTA暗号软复位进Bootloader
 History:
*******************************************************/

#ifndef __BSP_USART_H_
#define __BSP_USART_H_

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "app_task.h"

extern uint8_t rx_temp_u3;             // USART3单字节接收缓存
extern uint8_t UART3_Rxbuff[8];       // 上位机MODBUS帧快照：0x5B+功能码+2字节地址+2字节数据+CRC16(LSB,MSB)
extern volatile uint8_t HOST_Cmd_ready;  // 写命令帧(0x06)接收完成且CRC校验通过标志，主循环处理
extern volatile uint8_t HOST_Read_ready; // 读命令帧(0x03)接收完成且CRC校验通过标志，主循环处理

void MCU_HOST_Send(uint8_t *buf, uint8_t len);
void Go_To_Bootloader(void);

#endif /* __BSP_USART_H_ */
