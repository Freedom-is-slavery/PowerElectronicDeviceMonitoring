#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "lcd.h"
#include "sram.h"
#include "rtc.h"
#include "beep.h"
#include "adc.h"
#include "sram.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "tcp_client_demo.h"
#include "udp_demo.h"

////////////////////////////////////////////////////////////////////////////////
//	PROJECT: 基于Ethernet通信的电力电子设备状态的实时监测系统设计
//	CONTRIBUTORS: WWZ, ZGCL, WY
//	BRIEF INTRODUCTION: 该部分程序实现STM32上的AD转换和用LWIP协议实现Ethernet通信
///////////////////////////////////////////////////////////////////////////////


//标记当前使用的传输协议状态,并且可以指示该协议的准备是否就绪
//STATUS_NOT_ALREADY: 未选用传输方式或通信未准备就绪
//STATUS_IS_UDP: 启用UDP传输,且发送准备已就绪
//STATUS_IS_TCP_CLIENT: 启用TCP客户端传输,且发送准备已就绪(已完成握手建立TCP连接) 
u8 TestStatus = STATUS_NOT_ALREADY;

/**
 * @brief 加载引导界面UI
 * @param mode bit0: 0,不加载; 1,加载前半部分UI 
 * 			   bit1: 0,不加载; 1,加载后半部分UI
 **/
void lwip_test_ui(u8 mode)
{
	u8 speed;
	u8 buf[30]; 
	if (mode & 1<<0)
	{
		LCD_Fill(30, 30, lcddev.width, 110, WHITE);				//清除显示
		POINT_COLOR = RED;
		LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
		LCD_ShowString(30,50,200,16,16,"Power Electronic Device Monitor"); 	
	}
	if (mode & 1<<1)
	{
		LCD_Fill(30, 110, lcddev.width, lcddev.height, WHITE);	//清除显示
		LCD_ShowString(30, 110, 200, 16, 16, "lwIP Init Successed");
		if (lwipdev.dhcpstatus == 2)
			sprintf((char*)buf,"DHCP IP: %d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//打印动态IP地址
		else 
			sprintf((char*)buf,"Static IP: %d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//打印静态IP地址
		
		LCD_ShowString(30,130,210,16,16,buf); 
		
		speed = LAN8720_Get_Speed();		//得到网速
		if (speed&1<<1)
			LCD_ShowString(30,150,200,16,16,"Ethernet Speed:100M");
		else 
			LCD_ShowString(30,150,200,16,16,"Ethernet Speed:10M");
		
		LCD_ShowString(30,170,200,16,16,"KEY0:TCP Server Test");
		LCD_ShowString(30,190,200,16,16,"KEY1:TCP Client Test");
		LCD_ShowString(30,210,200,16,16,"KEY2:UDP Test");
	}
}

int main(void)
{
	u8 t;
	u8 key;
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);   	//串口波特率设置
	usmart_dev.init(84); 	//初始化USMART
	LED_Init();  			//LED初始化
	KEY_Init();  			//按键初始化
	LCD_Init(); 			//LCD初始化
	FSMC_SRAM_Init();		//初始化外部SRAM  
	BEEP_Init();			//蜂鸣器初始化
	//My_RTC_Init();  		//RTC初始化
	Adc_Init();  			//ADC初始化,其中包含ADC专用的DMA的初始化 
	//TIM2_Int_Init(100-1, 840-1);	//定时1ms,1000hz的采样率应该够用吧(狗头)
	TIM3_Int_Init(1000-1, 840-1); //100khz的频率,计数1000为10ms
	
	mymem_init(SRAMIN);		//初始化内部内存池
	mymem_init(SRAMEX);		//初始化外部内存池
	mymem_init(SRAMCCM);	//初始化CCM内存池
	
	POINT_COLOR = RED; 		//红色字体
	lwip_test_ui(1);		//加载前半部分UI
	
	//先初始化lwIP(包括LAN8720初始化),此时必须插上网线,否则初始化会失败!! 
	LCD_ShowString(30,110,200,16,16,"lwIP Initing...");
	while(lwip_comm_init()!=0)
	{
		LCD_ShowString(30,110,200,16,16,"lwIP Init failed!");
		delay_ms(1200);
		LCD_Fill(30,110,230,110+16,WHITE);//清除显示
		LCD_ShowString(30,110,200,16,16,"Retrying...");  
	}
	LCD_ShowString(30,110,200,16,16,"lwIP Init Successed");
	//等待DHCP获取 
 	
	LCD_ShowString(30,130,200,16,16,"DHCP IP configing...");
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{
		lwip_periodic_handle();
	}
	lwip_test_ui(2);//加载后半部分UI
	
	while(1)
	{
		key=KEY_Scan(0);
		switch(key)
		{
			case KEY1_PRES:			//TCP_Client模式
				tcp_client_test();
				lwip_test_ui(3);
				break;
			case KEY2_PRES:			//UDP模式
				udp_demo_test();
				lwip_test_ui(3);	//重新加载UI
				break; 
		}
		TestStatus = 0;
		lwip_periodic_handle();
		delay_ms(2);

		t++;
		if(t==100)
		{
			LCD_ShowString(30,230,200,16,16,"PRESS KEY2 TO START");
		}

		if(t==200)
		{ 
			t=0;
			LCD_Fill(30,230,230,230+16,WHITE);//清除显示
			LED0=!LED0;
		} 
	}
}








