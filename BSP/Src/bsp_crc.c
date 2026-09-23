


/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_crc.c
 Author: PAN     Version: V1.0       Date:2026/04/23
 Description:       CRC校验文件

 Function List:     CRC16_Modbus
 History:
*******************************************************/




#include "bsp_crc.h"

// 标准 Modbus CRC16 计算 (完美契合 8005, FFFF, REFIN, REFOUT)
uint16_t CRC16_Modbus(uint8_t *puchMsg, uint16_t usDataLen) 
{
    uint16_t wCRCin = 0xFFFF;
    uint16_t wCPoly = 0xA001; // 0x8005 的反转多项式
    uint8_t wChar = 0;
    
    for (uint16_t i = 0; i < usDataLen; i++) 
    {
        wChar = puchMsg[i];
        wCRCin ^= wChar;
        for (int j = 0; j < 8; j++) 
        {
            if (wCRCin & 0x01) {
                wCRCin >>= 1;
                wCRCin ^= wCPoly;
            } else {
                wCRCin >>= 1;
            }
        }
    }
    return wCRCin;
}




