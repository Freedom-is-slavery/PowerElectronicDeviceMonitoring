#ifndef _ADC_H
#define _ADC_H
#include "sys.h"

#define ADC_CH5  5  //通道5
#define ADC_ChannelNumber 6         //6个通道
#define ADC_SamplingTimes 10        //每轮采样10次

//AD采样值
// #define ADValue[0] PhaseA_Voltage
// #define ADValue[1] PhaseB_Voltage
// #define ADValue[2] PhaseC_Voltage
// #define ADValue[3] PhaseA_Current
// #define ADValue[4] PhaseB_Current
// #define ADValue[5] PhaseC_Current

void Adc_Init(void); 	//ADC通道初始化
void DMA2_Init(void);   //DMA2初始化

void Start_ADC_Conversion(void);    //开启ADC转换并等待转换完成

#define Phase_A 1
#define Phase_B 2
#define Phase_C 3
#define Type_Voltage 1
#define Type_Current 2
vu16 Get_ADC_Value(u8 phase, u8 SignalType);   

#endif

