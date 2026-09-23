
/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_iwdg.c
 Author: PAN        Version: V1.0       Date:2026/09/18
 Description:       独立看门狗（IWDG，2s超时）

 Function List:     IWDG_ON()      开启看门狗
                    IWDG_Refresh() 喂狗
 History:
*******************************************************/

#include "bsp_iwdg.h"
#include "stdio.h"

  void IWDG_Refresh()
  {
        HAL_IWDG_Refresh(&hiwdg); //刷新独立看门狗
  }

  void IWDG_ON()
  {
        MX_IWDG_Init(); //开启看门狗
  }
