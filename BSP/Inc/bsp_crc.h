/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_crc.h
 Author: PAN      Version: V1.0       Date:2026/04/23
 Description:       CRC16校验

 Function List:
 History:
*******************************************************/

#ifndef __BSP_CRC_H__
#define __BSP_CRC_H__
#include "main.h"

uint16_t CRC16_Modbus(uint8_t *puchMsg, uint16_t usDataLen);

#endif