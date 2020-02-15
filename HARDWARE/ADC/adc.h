#ifndef _ADC_H
#define _ADC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//ADC ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

#define ADC_CH5  5  //ͨ��5
#define ADC_ChannelNumber 6         //6��ͨ��
#define ADC_SamplingTimes 10        //ÿ�ֲ���10��

//AD����ֵ
// #define ADValue[0] PhaseA_Voltage
// #define ADValue[1] PhaseB_Voltage
// #define ADValue[2] PhaseC_Voltage
// #define ADValue[3] PhaseA_Current
// #define ADValue[4] PhaseB_Current
// #define ADValue[5] PhaseC_Current

void Adc_Init(void); 	//ADCͨ����ʼ��
void DMA2_Init(void);   //DMA2��ʼ��

void Start_ADC_Conversion(void);    //����ADCת�����ȴ�ת�����

#define Phase_A 1
#define Phase_B 2
#define Phase_C 3
#define Type_Voltage 1
#define Type_Current 2
vu16 Get_ADC_Value(u8 phase, u8 SignalType);   

#endif

