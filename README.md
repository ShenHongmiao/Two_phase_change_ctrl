# 两相变化控制系统 (Two-Phase Change Control System)

## 项目概述

本项目是基于 STM32F407 微控制器的**液气两相变化温度控制系统**。通过精确的温度和压力监测，实现对液体-气体相变过程的智能控制。

**当前状态**: 🚧 开发中 - 已完成传感器数据采集和通信模块，控制算法部分待开发。

## 硬件平台

- **MCU**: STM32F407VGT6
- **RTOS**: FreeRTOS
- **编译工具链**: GCC ARM None EABI
- **构建系统**: CMake + Ninja

## 核心功能模块

### 1. 温度检测 - NTC 热敏电阻

**硬件配置**:

- ADC1 双通道采集 (PA0, PA1)
- 10kΩ NTC 热敏电阻 (B值: 3380)
- 10kΩ 串联电阻
- 12位 ADC 分辨率

**技术特性**:

- DMA 自动数据采集
- Beta 公式温度计算
- 双通道支持（可通过宏配置使能）

**数据输出**: 通过 `send2pc(CMD_NTC, ...)` 发送二进制帧，可根据宏定义携带一个或两个通道温度值（`int16_t`，单位为℃×100）。

### 2. 温压检测 - WF5803F 传感器

**硬件配置**:

- I2C 总线通信 (地址: 0x6C)
- 2BAR 压力量程
- 温度 + 压力一体化传感器

**技术特性**:

- I2C DMA 非阻塞读取
- 压力和温度同步采集
- 适用于相变过程监测

**数据输出**: `send2pc(CMD_WF5803F, ...)` 发送温度（`int16_t`，℃×100）和压力（`int32_t`，kPa×100）。

### 3. 电压监测

**硬件配置**:

- ADC2 单通道采集 (PC4)
- 分压电路监测系统电压

**技术特性**:

- 低压报警功能
- 定时检测（10分钟周期）
- 状态标识（OK/LOW）

**数据输出**: `send2pc(CMD_VOLTAGE, ...)` 发送电压值（`int16_t`，V×100）与状态字节（正常 `0x01`，低压 `0xFF`）。

### 4. 上位机通信协议 (V1.0)

#### 4.1 基础通信参数

| 参数 | 值/描述 | 备注 |
| :--- | :--- | :--- |
| 字节序 (Endianness) | **小端序 (Little-Endian)** | 多字节数据类型（如 `int16_t`, `int32_t`）按低字节在前解析。 |
| 数据缩放 | **×100** | 所有浮点量通过乘以 100 后转换为整数发送，还原时需除以 100.0f。 |
| 校验算法 | **CRC8** | 多项式 `x^8 + x^2 + x + 1`，覆盖帧头到数据体。 |

#### 4.2 统一帧结构

上位机应以 `FRAME_HEAD` (`0xDE`) 和 `FRAME_TAIL` (`0xED`) 为界限捕获数据包：

| 偏移量 | 长度 | 字段名 | 类型 | 描述 |
| :---: | :---: | :---: | :---: | :--- |
| 0 | 1 | `FRAME_HEAD` | `uint8_t` | 固定值 `0xDE` |
| 1 | 1 | `cmd_id` | `uint8_t` | 命令 ID，决定数据体内容 |
| 2 | 1 | `data_len` | `uint8_t` | 数据体长度 N (0–255) |
| 3 | N | `payload` | 变长 | 有效负载，格式见下节 |
| 3+N | 1 | `crc8` | `uint8_t` | 对偏移 0 至 2+N 字节计算 |
| 4+N | 1 | `FRAME_TAIL` | `uint8_t` | 固定值 `0xED` |

#### 4.3 命令 ID 数据体解析

| 命令 ID | 宏定义 | 描述 |
| :---: | :---: | :--- |
| `0x01` | `CMD_NTC` | NTC 温度数据（支持 0–2 个通道，按宏启用） |
| `0x02` | `CMD_WF5803F` | WF5803F 传感器温/压数据 |
| `0x03` | `CMD_VOLTAGE` | 系统电压与状态 |
| `0x0F` | `CMD_TEXT_INFO` | 文本/诊断信息 |

**CMD `0x01` (`CMD_NTC`)：NTC 温度**

| 偏移量 | 长度 | 类型 | 描述 |
| :---: | :---: | :---: | :--- |
| 0 | 2 (可选) | `int16_t` | `ntc_temp_ch0`，当 `NTC_CHANNEL0_ENABLE == 1` 时存在 |
| 2 | 2 (可选) | `int16_t` | `ntc_temp_ch1`，当 `NTC_CHANNEL1_ENABLE == 1` 时存在 |

温度值放大 100 倍，换算：`deg_c = raw / 100.0f`。

**CMD `0x02` (`CMD_WF5803F`)：温度 & 压力**

| 偏移量 | 长度 | 类型 | 描述 |
| :---: | :---: | :---: | :--- |
| 0 | 2 | `int16_t` | 温度 (℃×100) |
| 2 | 4 | `int32_t` | 压力 (kPa×100) |

**CMD `0x03` (`CMD_VOLTAGE`)：电源电压**

| 偏移量 | 长度 | 类型 | 描述 |
| :---: | :---: | :---: | :--- |
| 0 | 2 | `int16_t` | 系统电压 (V×100) |
| 2 | 1 | `uint8_t` | 状态：正常 `0x01`，低压 `0xFF` |

**CMD `0x0F` (`CMD_TEXT_INFO`)：文本消息**

| 偏移量 | 长度 | 类型 | 描述 |
| :---: | :---: | :---: | :--- |
| 0 | N | `char[]` | 以 ASCII 编码的格式化字符串 (N ≤ 255) |

文本帧同样使用 `data_len` 标明字符串字节数，CRC 覆盖帧头至正文。上位机可直接按长度读取后进行显示或记录。

#### 4.4 上位机处理流程建议

1. 捕获帧：依据 `0xDE` 和 `0xED` 定位数据包边界。
2. 校验：对帧头至数据体进行 CRC8 计算，校验失败时丢弃。
3. 解析：根据 `cmd_id` 选择对应的数据结构并按小端序转换。
4. 还原：对所有缩放后的数值除以 100.0f，转换为实际物理量。

#### 4.5 通信技术特性

- **波特率**: 115200 bps
- **发送方式**: DMA 双缓冲非阻塞 (`send_binary_data`)，所有帧统一出口
- **接收方式**: 中断 + FreeRTOS 队列
- **硬件**: USART2 (PD5/PD6)
- **RS485 控制**: MD0=0, MD1=0

## FreeRTOS 任务架构

### 任务列表

| 任务名称 | 优先级 | 栈大小 | 周期 | 功能描述 |
|---------|-------|--------|------|---------|
| **DefaultTask** | Normal | 128 | 一次性 | 系统初始化，启动其他任务后自删除 |
| **MonitorTask** | Normal | 512 | 5ms | 采集温度/压力数据并发送 |
| **VoltageWarning** | Normal | 128 | 1000ms | 电压监测与报警 |
| **Receive_Target** | High | 256 | 事件驱动 | 接收上位机指令（预留） |
| **Ctrl_task** | High | 256 | TBD | 两相变化控制算法（待开发） |

### 任务流程图

```text
[DefaultTask] 初始化
    ├── 初始化外设 (NTC, Voltage, UART)
    ├── 发送系统启动消息
    ├── 恢复其他任务
    └── 自删除

[MonitorTask] 数据采集 (5ms周期)
    ├── ADC DMA 采集 NTC 数据
    ├── I2C DMA 读取 WF5803F 数据
    ├── 计算温度值
    └── UART 发送数据包

[VoltageWarning] 电压监测 (1000ms周期)
    ├── ADC 采集电压
    ├── 判断电压状态
    └── 发送电压信息

[Ctrl_task] 控制算法 (待开发)
    └── 基于温度/压力实现相变控制
```

## 项目结构

```text
Two_phase_change_ctrl/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # 主配置头文件
│   │   ├── usart.h             # 串口通信（协议定义）
│   │   ├── NTC.h               # NTC温度传感器
│   │   ├── WF5803F.h           # 温压传感器
│   │   ├── V_Detect.h          # 电压检测
│   │   ├── adc.h / dma.h       # ADC和DMA配置
│   │   ├── i2c.h               # I2C配置
│   │   └── FreeRTOSConfig.h    # RTOS配置
│   └── Src/
│       ├── main.c              # 主程序入口
│       ├── freertos.c          # FreeRTOS任务定义
│       ├── usart.c             # 串口通信实现
│       ├── NTC.c               # NTC温度计算
│       ├── WF5803.c            # WF5803F驱动
│       └── V_Detect.c          # 电压检测实现
├── Drivers/
│   ├── STM32F4xx_HAL_Driver/  # STM32 HAL库
│   └── CMSIS/                  # ARM CMSIS
├── Middlewares/
│   └── Third_Party/FreeRTOS/  # FreeRTOS源码
├── CMakeLists.txt              # CMake构建配置
├── CMakePresets.json           # CMake预设
└── README.md                   # 本文件
```

## 编译与烧录

### 编译

```bash
# 使用 CMake 配置
cmake --preset Debug

# 编译
cmake --build build/Debug
```

### 烧录

项目支持多种烧录方式：

#### 方法 1: OpenOCD + CMSIS-DAP

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg \
  -c "program build/Debug/Two_phase_change_ctrl.elf verify reset exit"
```

#### 方法 2: STM32CubeProgrammer

```bash
STM32_Programmer_CLI -c port=SWD -d build/Debug/Two_phase_change_ctrl.elf -rst
```

## 数据协议示例

### 启动输出

```text
DE 0F 0D 73 79 73 74 65 6D 20 73 74 61 72 74 36 79 ED   # [0x0F] "system start"
DE 01 04 04 B0 05 28 34 70 ED                            # [0x01] NTC CH0/CH1 (25.12℃ / 26.16℃)
DE 02 06 09 C4 00 01 8F 8C 3E 27 ED                      # [0x02] WF5803F 温压 (25.00℃, 101.26kPa)
DE 03 03 04 B0 01 4A 2B ED                               # [0x03] 电压 + 状态 (24.00V, 正常)
```

## 性能指标

### 通信带宽优化

| 数据类型 | 旧格式(JSON) | 新二进制帧 | 节省率 |
|---------|-------------|-------------|-------|
| NTC温度 | 47 字节 | 8 字节 (双通道) | 83% |
| WF5803F | 66 字节 | 9 字节 | 86% |
| 电压检测 | 64 字节 | 7 字节 | 89% |

### 实时性

- 温度采集周期: 5ms
- 电压检测周期: 1000ms
- UART DMA 发送: 非阻塞，双缓冲
- 最大数据延迟: < 10ms

## 待开发功能

- [ ] **相变控制算法** (Ctrl_task)
  - [ ] PID 温度控制
  - [ ] 压力-温度联合控制
  - [ ] 相变点识别与稳定

- [ ] **上位机指令接收** (Receive_Target)
  - [ ] 目标温度/压力设置
  - [ ] 控制参数调整
  - [ ] 系统状态查询

- [ ] **数据记录与存储**
  - [ ] Flash 存储历史数据
  - [ ] SD 卡日志记录

- [ ] **安全保护机制**
  - [ ] 超温保护
  - [ ] 超压保护
  - [ ] 异常状态处理

## 配置说明

### 宏定义配置

**NTC 通道使能** (`NTC.h`):

```c
#define NTC_CHANNEL0_ENABLE 1  // 1: 使能, 0: 禁用
#define NTC_CHANNEL1_ENABLE 0
```

**WF5803F 使能** (`WF5803F.h`):

```c
#define WF5803F_Enable 1  // 1: 使能, 0: 禁用
```

## 许可证

Copyright © 2025 STMicroelectronics.
All rights reserved.

本软件遵循 ST 许可协议。详见项目根目录 LICENSE 文件。

## 作者

ShenHongmiao

## 更新日志

### 2025-11-11

- ✅ 完成串口通信协议优化（JSON → 简洁前缀）
- ✅ 实现 NTC 双通道温度采集
- ✅ 集成 WF5803F 温压传感器
- ✅ 添加电压监测模块
- ✅ 更新通信耗时注释
- ✅ 创建项目文档

---

**项目目标**: 实现精确可靠的液气两相变化温度控制
