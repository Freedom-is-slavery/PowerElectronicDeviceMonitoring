#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//	UDP方式传输数据
////////////////////////////////////////////////////////////////////////////////// 	   
 
//UDP接收数据缓冲区
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];
//UDP发送数据缓冲区
u8 udp_demo_sendbuf[UDP_DEMO_TX_BUFSIZE];

u8 FileBuff[100]__attribute__((at(0x68040000)));

struct udp_pcb *udppcb;  	//UDP协议控制块(全局变量)

//UDP 测试全局状态标记变量
//bit7:没有用到
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上;1,连接上了.
//bit4~0:保留
u8 udp_demo_flag;

//设置远端IP地址
void udp_demo_set_remoteip(void)
{
	u8 *tbuf;

	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
	LCD_ShowString(30,50,200,16,16,"UDP Test");
	LCD_ShowString(30,70,200,16,16,"Remote IP Set");  

	tbuf=mymalloc(SRAMIN,100);	//申请内存
	if(tbuf==NULL)	return;

	//前三个IP保持和DHCP得到的IP一致
	//远端IP地址192.168.1.115
	lwipdev.remoteip[0]= lwipdev.ip[0];
	lwipdev.remoteip[1]= lwipdev.ip[1];
	lwipdev.remoteip[2]= lwipdev.ip[2]; 
	lwipdev.remoteip[3]= 115;

	//显示远端IP地址
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],\
			lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	LCD_ShowString(30,150,210,16,16,tbuf); 
	myfree(SRAMIN,tbuf); 
} 

//UDP测试
void udp_demo_test(void)
{
 	err_t err;
	struct ip_addr rmtipaddr;  	//远端ip地址
 	
	u8 *tbuf;					//LCD显示缓存指针
 	u8 key;
	u8 res=0;		
	u8 t=0; 

	udp_demo_set_remoteip();	//设置远端(计算机)IP地址
	
	//LCD UI界面
	LCD_Clear(WHITE);	//清屏
	POINT_COLOR=RED; 	//红色字体
	LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
	LCD_ShowString(30,50,200,16,16,"UDP Test");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");  
	LCD_ShowString(30,90,200,16,16,"KEY0:Send data");  
	LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit");  
	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL) return;		//内存申请失败了,直接退出
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
	LCD_ShowString(30,130,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
	LCD_ShowString(30,150,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote Port:%d",UDP_DEMO_PORT);//客户端端口号
	LCD_ShowString(30,170,210,16,16,tbuf);
	POINT_COLOR=BLUE;
	LCD_ShowString(30,190,210,16,16,"STATUS:Disconnected"); 

	udppcb=udp_new();
	if(udppcb)//创建成功
	{
		//此时res=0
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
		err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP客户端连接到指定IP地址和端口号的服务器
		if(err==ERR_OK)
		{
			err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//绑定本地IP地址与端口号
			if(err==ERR_OK)	//绑定完成
			{
				udp_recv(udppcb,udp_demo_recv,NULL);//注册接收回调函数 
				LCD_ShowString(30,190,210,16,16,"STATUS:Connected   ");//标记连接上了(UDP是非可靠连接,这里仅仅表示本地UDP已经准备好)
				POINT_COLOR=RED;
				LCD_ShowString(30,210,lcddev.width-30,lcddev.height-190,16,"Receive Data:");//提示消息		
				POINT_COLOR=BLUE;//蓝色字体
			}
			else res=1;
		}
		else res=1;		
	}
	else res=1;

	while(res==0)
	{
		if(udp_demo_flag&1<<6)//是否收到数据?
		{
		//接收数据处理部分,暂不使用

			udp_demo_flag&=~(1<<6);//标记数据已经被处理了	
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==200)
		{
			t=0;
			LED0=!LED0;				//LED0标识UDP进程正在运行
		}
	}

	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 


//UDP服务器回调函数
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p != NULL)	//接收到不为空的数据时
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
		for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
		{
			//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
			//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出UDP客户端接收数组,跳出	
		}
		upcb->remote_ip=*addr; 				//记录远程主机的IP地址
		upcb->remote_port=port;  			//记录远程主机的端口号
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;				//标记接收到数据了
		pbuf_free(p);//释放内存
	}
	else
	{
		udp_disconnect(upcb); 
	} 
} 
//UDP服务器发送数据,该函数用于发送应答帧
void udp_demo_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT, 9, PBUF_POOL); //申请内存
	if(ptr)
	{
		ptr->payload=(void*)udp_demo_sendbuf; 	//发送的数据在这里
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
	} 
} 
//关闭UDP连接
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);		//断开UDP连接 
}
























