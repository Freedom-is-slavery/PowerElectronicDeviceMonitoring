#include "timer.h"
#include "led.h"
#include "lwip_comm.h"
#include "adc.h"
#include "udp_demo.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

extern u32 lwip_localtime;
extern vu16 ADValue[ADC_ChannelNumber];

TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

void TIM2_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  			//ʹ��TIM2ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  					//��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   					//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); 						//����ʱ��2�����ж�
	TIM_Cmd(TIM2,ENABLE); 											//ʹ�ܶ�ʱ��2
	
	//�Զ�ʱ��2��NVIC����
		NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; 					//��ʱ��2�ж�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; 		//��ռ���ȼ�2,����TIM3���ȼ�
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 			//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  			//ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc; 					//��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   					//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); 						//����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); 											//ʹ�ܶ�ʱ��3
	
	//�Զ�ʱ��3��NVIC����
		NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; 					//��ʱ��3�ж�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; 		//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 			//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
}

//��ʱ��2�жϷ�����
//����ADC��ʱת���ʹ�������
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET) //����ж�
	{
		Start_ADC_Conversion();
		udp_demo_send_ADCValue(ADValue);
	}
}

//��ʱ��3�жϷ�����
//����lwip��ʱ
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		lwip_localtime +=10; //��10
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
