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

**数据格式**: `[01]CH0:25.36` (15字节, ~1.30ms @115200)

### 2. 温压检测 - WF5803F 传感器

**硬件配置**:

- I2C 总线通信 (地址: 0x6C)
- 2BAR 压力量程
- 温度 + 压力一体化传感器

**技术特性**:

- I2C DMA 非阻塞读取
- 压力和温度同步采集
- 适用于相变过程监测

**数据格式**: `[02]T:25.36,P:101.32` (23字节, ~2.00ms @115200)

### 3. 电压监测

**硬件配置**:

- ADC2 单通道采集 (PC4)
- 分压电路监测系统电压

**技术特性**:

- 低压报警功能
- 定时检测（10分钟周期）
- 状态标识（OK/LOW）

**数据格式**: `[03]V:12.50,OK` (15字节, ~1.30ms @115200)

### 4. 串口通信协议

#### 数据包格式

采用**简洁前缀协议**，相比 JSON 格式节省约 70% 带宽：

```
[ID]数据内容\n
```

#### 命令 ID 定义

| ID | 类型 | 格式示例 | 字节数 | 传输时间@115200 |
|---|---|---|---|---|
| `0x01` | NTC温度 | `[01]CH0:25.36\n` | 15 | ~1.30ms |
| `0x02` | WF5803F温压 | `[02]T:25.50,P:101.32\n` | 23 | ~2.00ms |
| `0x03` | 电压检测 | `[03]V:12.50,OK\n` | 15 | ~1.30ms |
| `0xFF` | 系统信息 | `[FF]system start\n` | 18 | ~1.56ms |

#### 通信技术特性

- **波特率**: 115200 bps
- **发送方式**: DMA 双缓冲非阻塞
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

```
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

```
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

**方法 1: OpenOCD + CMSIS-DAP**

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg \
  -c "program build/Debug/Two_phase_change_ctrl.elf verify reset exit"
```

**方法 2: STM32CubeProgrammer**

```bash
STM32_Programmer_CLI -c port=SWD -d build/Debug/Two_phase_change_ctrl.elf -rst
```

## 数据协议示例

### 启动输出

```
[FF]system start
[01]CH0:25.36
[02]T:25.50,P:101.32
[03]V:12.50,OK
```

## 性能指标

### 通信带宽优化

| 数据类型 | 旧格式(JSON) | 新格式(前缀) | 节省率 |
|---------|-------------|-------------|-------|
| NTC温度 | 47字节 | 15字节 | 68% |
| WF5803F | 66字节 | 23字节 | 65% |
| 电压检测 | 64字节 | 15字节 | 77% |

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
