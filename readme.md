# 基于以太网通信的电力电子设备的远程监控嵌入式系统的设计

本项目是基于搭载有STM32F407ZGT6微处理器的开发板设计的电力电子设备的远程监测系统

## 系统框架

### 1 信号采样和调理电路

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
- 多通道复用ADC1进行转换,采用单次、扫描模式
- 采样时间设置(时间越短精度越低)
- DMA配置(用法见于**STM32F4xx中文参考手册11.8**)
- 查询方式判断ADC转换是否完成

### 2.2 LWIP框架下实现Ethernet通信

IP层的基本配置位于 lwip_comm.c 中:

(已经默认开启DHCP服务，超时重试不成功后将使用默认配置)

- 默认远端IP: 192.168.1.115 (可在udp_demo.c中定义的udp_demo_set_remoteip中修改)

- 默认本地IP: 192.168.1.30

运输层的用户配置:

- UDP(在udp_demo.c/.h中修改)
  - Remote Port = 8089
  - Local Port = 8089
- TCP Client
- TCP Server

### 3 PC端监控界面设计

## 工程文件的结构

- CORE: CMSIS(Cortex MicroController Software Interface Standard)软件接口层
  - 核内外设访问层(core_cm4.h)
  - 中间件访问层
  - 外设访问层
  - ...

- FWLIB: Firmware Library,官方固件库

- HARDWARE: 硬件驱动和基本配置

- LWIP: Lightweight IP协议栈 ([More](https://savannah.nongnu.org/projects/lwip/))
  - arch
  - lwip_app: 用户自定义应用
  - lwip-1.4.1

- MALLOC: 内存管理

## 其他

[我的GitHub主页](https://github.com/Freedom-is-slavery)

参考资料：

1. STM32F4技术手册

2. Alientek开发板相关手册

3. 计算机网络与通信体系

4. LWIP协议栈源代码
