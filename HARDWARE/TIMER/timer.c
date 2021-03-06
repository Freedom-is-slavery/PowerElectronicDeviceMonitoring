#include "timer.h"
#include "led.h"
#include "lwip_comm.h"
#include "adc.h"
#include "udp_demo.h"
#include "tcp_client_demo.h"

extern u32 lwip_localtime;
extern vu16 ADValue[ADC_ChannelNumber];
int16_t ADValueTest[ADC_ChannelNumber];
int16_t SinValue[20] = {0, 96, 175, 252, 296, 326, 296, 252, 183, 96, 0, -96, -183, -252, -286, -316, -296, -252, -189, -88};
extern u8 TestStatus;
u16 cnt;

TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

void TIM2_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  			//使能TIM2时钟
	//定时器2配置
		TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  					//定时器分频
		TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//向上计数模式
		TIM_TimeBaseInitStructure.TIM_Period=arr;   					//自动重装载值
		TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
		TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
		TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); 						//允许定时器2更新中断
		TIM_Cmd(TIM2,ENABLE); 											//使能定时器2
	//对定时器2的NVIC配置
		NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; 					//定时器2中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; 		//抢占优先级2,低于TIM3优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 			//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

//通用定时器3中断初始化
//arr：自动重装值
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
void TIM3_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  			//使能TIM3时钟
	//定时器3配置
		TIM_TimeBaseInitStructure.TIM_Prescaler=psc; 					//定时器分频
		TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//向上计数模式
		TIM_TimeBaseInitStructure.TIM_Period=arr;   					//自动重装载值
		TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
		TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
		TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); 						//允许定时器3更新中断
		TIM_Cmd(TIM3,ENABLE); 											//使能定时器3	
	//对定时器3的NVIC配置
		NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; 					//定时器3中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; 		//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 			//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief 定时器2中断服务函数，用于ADC定时转换和处理数据
 **/
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET) //溢出中断
	{
		// ADValueTest[0] = SinValue[cnt];
		// ADValueTest[1] = SinValue[(cnt+7)%20];
		// ADValueTest[2] = SinValue[(cnt+14)%20];
		ADValueTest[3] = SinValue[cnt];
		ADValueTest[4] = SinValue[(cnt+7)%20];
		ADValueTest[5] = SinValue[(cnt+14)%20];

		udp_demo_send_ADCValue((int16_t *)ADValueTest);
		if ((++cnt) == 20)
		{
			cnt = 0;
		}
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}

//定时器3中断服务函数
//用于lwip定时
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		lwip_localtime +=10; //加10
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}
