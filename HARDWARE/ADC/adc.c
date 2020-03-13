#include "adc.h"
#include "delay.h"

//*************************************
//
// 实现三相电压电流的实时采样和转换
// 
//*************************************

vu16 ADValue[ADC_ChannelNumber];	//通过DMA方式存储的一组AD转换结果

//初始化ADC
//使用6个规则通道,循环扫描模式
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

	//时钟初始化
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);  //使能ADC1时钟
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOC时钟
	//初始化DMA	
		DMA2_Init();
	//GPIO初始化
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; //模拟输入模式
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
		//PA4,PA5,PA6 初始化
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
		//PC0,PC2,PC3 初始化
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_Init(GPIOC,&GPIO_InitStructure);
	//ADC通用初始化
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; 	//独立模式
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; //ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
		//ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //两个采样之间的时间间隔 双重或三重交错模式下时使用
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; //使用DMA
		ADC_CommonInit(&ADC_CommonInitStructure);
	//ADC1初始化
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;  //12位分辨率
		ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //扫描模式
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //连续转换模式
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //规则通道外部触发边沿选择,此处未用到
		//ADC_InitStructure.ADC_ExternalTrigConv =  //为常规通道组选择外部事件,此处未用到
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //数据右对齐
		ADC_InitStructure.ADC_NbrOfConversion = ADC_ChannelNumber; //6个转换在规则序列中
		ADC_Init(ADC1,&ADC_InitStructure); 
	//ADC_DMA使能
		ADC_DMARequestAfterLastTransferCmd(ADC1,DISABLE);	
		ADC_DMACmd(ADC1,ENABLE);
	//规则通道组设置
	//每个通道采样时间暂定为15个周期,ADCCLK频率21M
	//采样时间过短可能会影响转换精确度和DMA传输
		ADC_RegularChannelConfig(ADC1,ADC_Channel_4,1,ADC_SampleTime_15Cycles); 	
		ADC_RegularChannelConfig(ADC1,ADC_Channel_5,2,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_6,3,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_10,4,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_12,5,ADC_SampleTime_15Cycles);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_13,6,ADC_SampleTime_15Cycles);
	//使能ADC1
		ADC_Cmd(ADC1,ENABLE);
	//校准与复位ADC1
		//ADC_ResetCalibration(ADC1); 					//复位指定的ADC1的校准寄存器
 		//while(ADC_GetResetCalibrationStatus(ADC1)); 	//获取ADC1复位校准寄存器的状态,设置状态则等待
 		//ADC_StartCalibration(ADC1); 					//开始指定ADC1的校准状态
 		//while(ADC_GetCalibrationStatus(ADC1)); 		//获取指定ADC1的校准程序,设置状态则等待
	
}

void DMA2_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);	 //使能DMA2时钟
	while(DMA_GetCmdStatus(DMA2_Stream0) != DISABLE);

	DMA_Cmd(DMA2_Stream0, DISABLE);
	DMA_DeInit(DMA2_Stream0);	//将DMA的通道1所有配置重设为缺省值
	
	//DMA控制初始化结构体配置	
		DMA_InitStructure.DMA_Channel = DMA_Channel_0;	//ADC1专用的通道0
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;	//DMA外设ADC数据寄存器ADC_DR地址
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)ADValue;	//DMA内存基地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	//传输方向从外设到内存
		DMA_InitStructure.DMA_BufferSize = ADC_ChannelNumber;	//DMA缓存大小
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设ADC数据寄存器地址不变
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址寄存器递增
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //!!可能需要修改!! 工作在循环缓存模式
		DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道1拥有高优先级
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //DMA通道1没有设置为内存到内存传输
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;	//FIFO阈值
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;	//单次传输
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	//单次传输
		DMA_Init(DMA2_Stream0, &DMA_InitStructure); //根据DMA_InitStruct中指定的参数初始化DMA的通道
	
	DMA_Cmd(DMA2_Stream0, ENABLE);	//使能DMA2数据流0,启动传输
}

//开启ADC转换和DMA传输,等待转换完成和DMA传输完成
//存在的问题: ADC规则通道一组序列转换完成后,
//1. DMA传输开始的时间? A: 在每一个通道完成转换后,就启动一次传输
//2. 新一轮AD转换开始后, DMA传输目标内存单元的基地址是否重置? A:尚不明确
//3. 如何开始新一轮的转换和传输?
void Start_ADC_Conversion(void)
{
	u8 i;
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);
	ADC_SoftwareStartConv(ADC1); //开启转换,此处置位的是ADC1_CR2的SWSTART位
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);	//等待转换完成	
	while(DMA_GetFlagStatus(DMA2_Stream0,DMA_FLAG_TCIF0) != RESET);	//!!可能需要修改!! 等待DMA传输完成
	//return ADC_GetConversionValue(ADC1);//返回转换结果,读取的是DR寄存器值

	for(i = 1; i <= ADC_ChannelNumber; i++)
	{
		ADValue[i] = ADValue[i] * 3.3 / 4096;	//电压值转换
	} 
}
	
 





