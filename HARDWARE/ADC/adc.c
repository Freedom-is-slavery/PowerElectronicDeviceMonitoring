#include "adc.h"
#include "delay.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"

//*************************************
//
// ʵ�������ѹ������ʵʱ������ת��
// 
//*************************************

#define AD_Value[0] PhaseA_Voltage
#define AD_Value[1] PhaseB_Voltage
#define AD_Value[2] PhaseC_Voltage
#define AD_Value[3] PhaseA_Current
#define AD_Value[4] PhaseB_Current
#define AD_Value[5] PhaseC_Current

vu16 AD_Value[ADC_ChannelNumber];	//ͨ��DMA��ʽ�洢��һ��ADת�����

//��ʼ��ADC
//ʹ��6������ͨ��,ѭ��ɨ��ģʽ
// - PA4 -> Channel 4
// - PA5 -> Channel 5
// - PA6 -> Channel 6
// - PC0 -> Channel 10
// - PC2 -> Channel 12
// - PC3 -> Channel 13
void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//ʱ�ӳ�ʼ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);  //ʹ��ADC1ʱ��
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOCʱ��
	//��ʼ��DMA	
		DMA2_Init();
	//GPIO��ʼ��
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; //ģ������ģʽ
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
		//PA4,PA5,PA6 ��ʼ��
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
		//PC0,PC2,PC3 ��ʼ��
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_Init(GPIOC,&GPIO_InitStructure);
	//ADCͨ�ó�ʼ��
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; 	//����ģʽ
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; //ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz
		//ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //��������֮���ʱ���� ˫�ػ����ؽ���ģʽ��ʱʹ��
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; //ʹ��DMA
		ADC_CommonInit(&ADC_CommonInitStructure);
	//ADC1��ʼ��
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;  //12λ�ֱ���
		ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //ɨ��ģʽ
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //����ת��ģʽ
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //����ͨ���ⲿ��������ѡ��,�˴�δ�õ�
		//ADC_InitStructure.ADC_ExternalTrigConv =  //Ϊ����ͨ����ѡ���ⲿ�¼�,�˴�δ�õ�
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //�����Ҷ���
		ADC_InitStructure.ADC_NbrOfConversion = ADC_ChannelNumber; //6��ת���ڹ���������
		ADC_Init(ADC1,&ADC_InitStructure); 
	//ADC_DMAʹ��
		ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);	//Դ���ݱ仯ʱ����DMA����
		ADC_DMACmd(ADC1,ENABLE);
	//����ͨ��������
	//ÿ��ͨ������ʱ���ݶ�Ϊ15������,ADCCLKƵ��21M
	//����ʱ����ʵ�ʲ��Խ���ʵ��ӳ�,����߾�ȷ��
		ADC_RegularChannelConfig(ADC1,ADC_Channel_4,1,ADC_SampleTime_15Cycles); 	
		ADC_RegularChannelConfig(ADC1,ADC_Channel_5,2,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_6,3,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_10,4,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_12,5,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_13,6,ADC_SampleTime_15Cycles);
	//ʹ��ADC1
		ADC_Cmd(ADC1,ENABLE);
	//У׼�븴λADC1
		ADC_ResetCalibration(ADC1); 					//��λָ����ADC1��У׼�Ĵ���
 		while(ADC_GetResetCalibrationStatus(ADC1)); 	//��ȡADC1��λУ׼�Ĵ�����״̬,����״̬��ȴ�
 		ADC_StartCalibration(ADC1); 					//��ʼָ��ADC1��У׼״̬
 		while(ADC_GetCalibrationStatus(ADC1)); 			//��ȡָ��ADC1��У׼����,����״̬��ȴ�
	
}

void DMA2_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);	 //ʹ��DMA2ʱ��
	while(DMA_GetCmdStatus(DMA2_Stream0) != DISABLE);

	DMA_Cmd(DMA2_Stream0, DISABLE);
	DMA_DeInit(DMA_Channel_0);	//��DMA��ͨ��1������������Ϊȱʡֵ
	
	//DMA���Ƴ�ʼ���ṹ������	
		DMA_InitStructure.DMA_Channel = DMA_Channel_0;	//ADC1ר�õ�ͨ��0
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;	//DMA����ADC���ݼĴ���ADC_DR��ַ
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&AD_Value;	//DMA�ڴ����ַ
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	//���䷽������赽�ڴ�
		DMA_InitStructure.DMA_BufferSize = ADC_ChannelNumber;	//DMA�����С
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//����ADC���ݼĴ�����ַ����
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //�ڴ��ַ�Ĵ�������
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16λ
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //������ѭ������ģʽ
		DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ��1ӵ�и����ȼ�
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //DMAͨ��1û������Ϊ�ڴ浽�ڴ洫��
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull	//FIFO��ֵ
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;	//���δ���
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	//���δ���
		DMA_Init(DMA2_Stream0, &DMA_InitStructure); //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
	
	DMA_Cmd(DMA2_Stream0, ENABLE);	//ʹ��DMA2������0,��������
}



	ADC_SoftwareStartConv(ADC1); //����ת��
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);//�ȴ�ת�����
	return ADC_GetConversionValue(ADC1);//����ת�����
 





