#include "beep.h"

//初始化PF8为输出口
//BEEP IO初始化
void BEEP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); 	//使能GPIOF时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  				//PF8引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 			//输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 		//速度100M
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   			//下拉
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOF,GPIO_Pin_8); //PF8拉低
}
