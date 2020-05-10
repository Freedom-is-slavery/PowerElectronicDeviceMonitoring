# 基于以太网通信的电力电子设备的远程监控嵌入式系统的设计

本项目是电力电子装置运行状态的远程监测系统，实现电路的三相电压电流采样、信号调理、AD转换、以太网通信及PC端可视化监测，该Repository实现基于STM32F407ZGT6核心板开发的AD转换、电压电流指标分析和以太网通信。

> Zhejiang University SRTP(Student Research Training Program)

> 浙江大学本科生科研实践项目

> 指导：邓焰教授 Guided by: Professor Yan.Deng

## 硬件平台

![ ](.\Illustration\board.jpg)

[Alientek STM32F407ZGT6开发板](http://www.alientek.com/productinfo/714608.html)

- MCU: STM32F407ZGT6, 32位ARM-Cortex M4架构

## 系统框架

### 1 信号采样和调理电路

原理图和PCB使用Altium Designer绘制

### 2 AD转换和Ethernet通信

### 2.1 AD转换

#### 硬件资源分配

需要六个通道(A,B,C三相电压和电流),使用**ADC1**

- PA4 -> Channel 4
- PA5 -> Channel 5
- PA6 -> Channel 6
- PC0 -> Channel 10
- PC2 -> Channel 12
- PC3 -> Channel 13

#### 寄存器相关

- SAR ADC
- 规则通道和注入通道(本项目使用**规则通道**)
- 相关寄存器
  - ADC_CR1: 分辨率(RES), 开启扫描(SCAN), 中断标志位(EOCIE, JEOCIE), ...
  - ADC_CR2: ADC开关(ADON), 连续转换开关(CONT)
  - ADC_SQR: 规则通道转换组顺序和数量
  - ADC_JSQR: 注入通道转换组顺序和数量
  - ADC_SR: EOC, ...
  - ADC_DR: 存储AD转换结果

#### 编程相关

- GPIO和ADC时钟使能(**ADC时钟最好不要超过36MHz**)
- DMA时钟使能(DMA2挂在总线AHB1下,因此使能AHB1时钟)
- 多通道复用ADC1进行转换,采用单次、扫描模式
- 采样时间设置(时间越短精度越低)
- DMA配置(用法见于**STM32F4xx中文参考手册11.8**)
- 查询方式判断ADC转换是否完成

### 2.2 LWIP协议栈实现Ethernet通信

> UDP通信已经调试可实现，TCP通信未调试

Notes:

IP层的基本配置位于 lwip_comm.c 中:

(已经开启DHCP服务，超时重试不成功后将使用默认配置)

- 默认远端IP: 192.168.1.115 (可在udp_demo.c中定义的udp_demo_set_remoteip中修改)

- 默认本地IP: 192.168.1.30

运输层的用户配置:

- UDP(在udp_demo.c/.h中修改)
  - Remote Port = 8089
  - Local Port = 8089
- TCP Client
- TCP Server

### 3 PC端监控界面设计

使用LABVIEW 2018开发

## 工程文件的结构

- CORE: CMSIS(Cortex MicroController Software Interface Standard)软件接口层标准
  - 核内外设访问层(core_cm4.h)
  - 中间件访问层
  - 外设访问层
  - ...

- FWLIB: Firmware Library，STM32F4固件库，加入了[跟以太网相关的库文件](.\FWLIB\STM32F4x7_ETH_Driver)

- HARDWARE: 硬件驱动和功能

- LWIP: Lightweight IP协议栈
  
  [源码](https://savannah.nongnu.org/projects/lwip/)
  - arch
  - lwip_app: 用户自定义应用
  - lwip-1.4.1

- MALLOC: 内存管理

## TODO LIST

- TIM2定时修改
- ADC_DMA启动方式完善
- ADC采样周期调试和修改
- 功能裁剪,删除冗余
- 加入TCP通信功能
- 丢包测试
- ( 考虑加入OS调度 )
- ......

## 初期实现效果：

![ ](.\Illustration\example.jpg)

## 其他

### Reference

1. STM32F4xx Manual

2. Cortex M4 内核编程手册

3. Alientek开发板例程、资料等

4. 计算机网络与通信体系

5. LWIP协议栈源代码
