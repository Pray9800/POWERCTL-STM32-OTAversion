# JT004-POWERCTL-STM32-OTAversion

JT101 动力切换控制板固件（STM32F103RET6）。由 IAR 工程 `JT_101_stm32f103_ctrlboard_1` 迁移至 VSCode + CMake + CubeMX 开发方式，并集成 OTA 升级功能（复用 JT004 Bootloader 架构）。

根据 PC1 引脚的动力电压检测（电阻分压 + ADC），自动选择 12V / 15V 动力通道继电器输出；上位机通过 USART3（网口转串口模块）下发使能与查询指令。

---

## 1. Flash 分区（与 JT004 Bootloader 配套）

| 分区 | 地址 | 大小 | 说明 |
|---|---|---|---|
| Bootloader | 0x08000000 | 32KB | 使用 `../JT004_ROBOT_Bootloader` 工程编译产物 |
| APP（本工程） | 0x08008000 | 240KB | 链接脚本 `STM32F103XX_FLASH.ld`，向量表重定位 SCB->VTOR=0x08008000 |
| 下载区 | 0x08044000 | 238KB | OTA 固件暂存区 |
| 描述符页 | 0x0807F800 | 2KB | 16字节OTA包头：magic"JT04"+版本+长度+CRC16 |

## 2. 硬件资源配置

| 资源 | 引脚 | 用途 |
|---|---|---|
| USART3 | PB10(TX) / PB11(RX) | 网口模块通信，115200-8-N-1 |
| ADC1_IN11 | PC1 | 动力电压检测（分压后） |
| CTL_12 | PB0 | 继电器：CTL_SWITCH=2（15V通道）时闭合 |
| CTL_15 | PB1 | 继电器：CTL_SWITCH=1（12V通道）时闭合 |
| LED1 | PB3 | 状态指示（JTAG已禁用） |
| USR_RSET / USR_CFG | PB12 / PB13 | 网口模块复位/配置，输出高保持 |
| TIM2 | - | 0.5ms 中断：ADC 采样 + LED 节奏 |
| IWDG | - | 2s 超时，主循环喂狗 |

时钟：HSE 8MHz × PLL9 = 72MHz；ADC 时钟 12MHz（PCLK2/6）；ADC 采样周期 239.5。

## 3. 通信协议（USART3，8字节定长 MODBUS 帧）

```
[0]=0x5B  [1]=功能码  [2..3]=寄存器地址(大端)  [4..5]=数据  [6]=CRC16低  [7]=CRC16高
CRC16-MODBUS，计算范围：前6字节
```

| 寄存器 | 功能码 | 含义 | 应答 |
|---|---|---|---|
| 0x0009 | 0x06/0x03 | 心跳 | 原帧回显 |
| 0x0024 | 0x06 | 电源选择写：data[1]=使能（非0=输出开） | 原帧回显 |
| 0x0024 | 0x03 | 电源选择读 | `5B 03 00 24 00 <CTL_SWITCH> crcL crcH` |

通道变化时主动上报：`5B 03 00 24 00 <CTL_SWITCH> crcL crcH`。

## 4. ADC 阈值与 LED 指示（TIM2 0.5ms 中断）

- 采样周期 2.5ms，一阶 IIR 滤波（15/20 旧值 + 5/20 新值）
- 阈值（滤波后 ADC 值）：>4000 → CTL_SWITCH=0（断开）；>2000 → 1（12V）；>900 → 2（15V）
- LED 节奏（25ms 节拍）：未使能=25ms 快闪；12V 通道=250ms 翻转；15V 通道=1s 翻转

继电器切换时序：先断旧通道 → 延时 1s → 合新通道（防两路同时闭合）。

## 5. OTA 升级

1. 网口模块透传发送 ASCII 命令 `UPDATE_FIRMWARE`（UART3）
2. 设备写 BKP DR1=0x55AA 后软复位进 Bootloader（软复位会禁用软件模式 IWDG，升级期间无看门狗复位）
3. Bootloader 从 UART3 接收 16 字节包头（magic `JT04` + 版本 + 长度 + CRC16-MODBUS）校验后接收固件到下载区，CRC 校验通过后搬运至 APP 区并写描述符页
4. 升级失败（TIMEOUT / CRC FAIL）自动回退旧版本，需从 `UPDATE_FIRMWARE` 重新开始（不支持断点续传）
5. 升级完成后延迟约 10s 再查版本，发送 ASCII 命令 `VERSION` 应答 `FW-JT101-01 V1.2.0.20260917`

注意：升级文件必须二进制透传发送，禁止文本模式转换。

## 6. 构建方法

环境：CMake 3.22+、Ninja、arm-none-eabi-GCC（STM32CubeCLT 或独立工具链）。

```powershell
# 方式一：VSCode + CMake Tools 插件（.vscode/settings.json 已配置）
# 方式二：命令行
cmake --preset Debug          # 配置（产物目录 build/Debug）
cmake --build build/Debug     # 编译
```

产物：`build/Debug/JT004-POWERCTL-STM32-OTAversion.elf / .hex / .bin`

CubeMX：打开 `JT004-POWERCTL-STM32-OTAversion.ioc` 可查看全部外设配置，GENERATE 后重新编译即可。

## 7. 烧录

- 单独烧 APP：`build/Debug/JT004-POWERCTL-STM32-OTAversion.hex`（注意烧录前 Full Chip Erase，避免下载区/描述符页残留旧 OTA 数据触发自愈搬运）
- 合并烧录：用 hex_merge 工具将 Bootloader + APP 合并，烧录前必须全片擦除后硬断电重上电

## 8. 与 IAR 原版差异

| 项目 | IAR 原版 | 本工程 |
|---|---|---|
| 开发环境 | IAR + 标准外设库 | VSCode + CMake + CubeMX + HAL |
| OTA | 无 | 有（UPDATE_FIRMWARE / VERSION） |
| IWDG | 代码存在但被注释 | 启用（2s，主循环喂狗） |
| RS485(USART2) | 初始化但接收逻辑被注释 | 已移除（死代码不移植） |
| 帧处理位置 | TIM2 中断内 | bsp_usart 置标志，app_task 主循环执行 |
| 网口模块 PB12/PB13 | 未初始化 | 输出高保持（与 Bootloader 行为一致） |
