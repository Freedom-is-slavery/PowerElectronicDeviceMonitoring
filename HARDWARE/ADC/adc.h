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

void Adc_Init(void); 				//ADCͨ����ʼ��
void DMA2_Init(void);               //DMA2��ʼ��

#endif

