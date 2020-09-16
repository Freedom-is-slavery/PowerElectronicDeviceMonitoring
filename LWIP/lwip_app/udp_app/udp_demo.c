/**
 * @file udp_demo.c
 * @brief ��UDP��ʽֱ�ӷ�������
 * @note UDP��ʽ����ҪTCP�������ֽ������ӣ���˸����٣���Ҳ�����׶���
 **/

#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "adc.h"

u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ�����
u8 udp_demo_sendbuf[UDP_DEMO_TX_BUFSIZE];	//UDP�������ݻ�����
extern u8 TestStatus;
extern vu16 ADValue[ADC_ChannelNumber];
extern int16_t ADValueTest[ADC_ChannelNumber];

struct udp_pcb *udppcb;  	//UDPЭ����ƿ�

//UDP ����ȫ��״̬��Ǳ���
//bit7:û���õ�
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û��������;1,��������.
//bit4~0:����
u8 udp_demo_flag;

//����Զ��IP��ַ
void udp_demo_set_remoteip(void)
{
	u8 *tbuf;

	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
	LCD_ShowString(30,50,200,16,16,"UDP Test");
	LCD_ShowString(30,70,200,16,16,"Remote IP Set");  

	tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
	if(tbuf==NULL)	return;

	//ǰ����IP���ֺ�DHCP�õ���IPһ��
	//Զ��IP��ַ192.168.1.115
	lwipdev.remoteip[0]= lwipdev.ip[0];
	lwipdev.remoteip[1]= lwipdev.ip[1];
	lwipdev.remoteip[2]= lwipdev.ip[2]; 
	lwipdev.remoteip[3]= 115;

	//��ʾԶ��IP��ַ
	sprintf((char*)tbuf, "Remote IP: %d.%d.%d.%d", lwipdev.remoteip[0],\
			lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	LCD_ShowString(30,150,210,16,16,tbuf); 
	myfree(SRAMIN,tbuf); 
} 

//UDP����
void udp_demo_test(void)
{
 	err_t err;
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
 	
	u8 *tbuf;					//LCD��ʾ����ָ��
	u8 res = 0;		
	u16 t = 0; 

	udp_demo_set_remoteip();	//����Զ��(�����)IP��ַ
	
	LCD_Clear(WHITE);
	POINT_COLOR = RED;
	LCD_ShowString(30, 30, 200, 16, 16, "Running LWIP......");
	LCD_ShowString(30, 50, 200, 16, 16, "UDP Transmission");  
	LCD_ShowString(30, 90, 200, 16, 16, "KEY_UP:Quit");

	tbuf = mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf == NULL) 
		return;		//�ڴ�����ʧ����,ֱ���˳�
	
	POINT_COLOR = GRAY;
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	LCD_ShowString(30,130,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	LCD_ShowString(30,150,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote Port:%d",UDP_DEMO_PORT);//�ͻ��˶˿ں�
	LCD_ShowString(30,170,210,16,16,tbuf);

	POINT_COLOR = BLUE;
	LCD_ShowString(30, 190, 210, 16, 16, "STATUS:Disconnected"); 

	udppcb = udp_new();
	if(udppcb)	//�����ɹ�
	{
		IP4_ADDR(&rmtipaddr, lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);
		err = udp_connect(udppcb, &rmtipaddr, UDP_DEMO_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����
		if(err == ERR_OK)
		{
			err = udp_bind(udppcb, IP_ADDR_ANY,UDP_DEMO_PORT);//�󶨱���IP��ַ��˿ں�
			if(err==ERR_OK)		//�����
			{
				udp_recv(udppcb,udp_demo_recv,NULL);	//ע����ջص�����
				LCD_ShowString(30, 190, 210, 16, 16, "STATUS:Connected   ");//�����������(UDP�Ƿǿɿ�����,���������ʾ����UDP�Ѿ�׼����)
				POINT_COLOR = RED;
				LCD_ShowString(30, 210, lcddev.width-30, lcddev.height-190, 16, "RMS Voltage:");//��ʾ��Ϣ		
				POINT_COLOR = BLACK;

				TIM2_Int_Init(100-1, 840-1);	//����TIM2����ʼ1ms��ʱ��������ݷ���
			}
			else res = 1;
		}
		else res = 1;		
	}
	else res = 1;

	while(res == 0)
	{
		if(udp_demo_flag & 1<<6)//�Ƿ��յ�����?
		{
		//�������ݴ�����,�ݲ�ʹ��

			udp_demo_flag &= ~(1<<6);//��������Ѿ���������	
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t == 500)
		{
			t = 0;
			//sprintf((char*)tbuf, "Voltage A: %3d V", (u16)(ADValue[3] * 3.3 / 4096 * 800 / 3));
			sprintf((char*)tbuf, "Phase A: %4d V", (int16_t)(ADValueTest[3]));
			LCD_ShowString(30,230,210,16,16,tbuf);
			//sprintf((char*)tbuf, "Voltage B: %3d V", (u16)(ADValue[4] * 3.3 / 4096 * 800 / 3));
			sprintf((char*)tbuf, "Phase B: %4d V", (int16_t)(ADValueTest[4]));
			LCD_ShowString(30,250,210,16,16,tbuf);
			//sprintf((char*)tbuf, "Voltage C: %3d V", (u16)(ADValue[5] * 3.3 / 4096 * 800 / 3));
			sprintf((char*)tbuf, "Phase C: %4d V", (int16_t)(ADValueTest[5]));
			LCD_ShowString(30,270,210,16,16,tbuf);

			LED0 =! LED0;				//LED0��ʶUDP������������
		}
	}

	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 


//UDP���������ջص�������Ŀǰ�ò���
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p != NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����UDP�ͻ��˽�������,����	
		}
		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;				//��ǽ��յ�������
		pbuf_free(p);//�ͷ��ڴ�
	}
	else
	{
		udp_disconnect(upcb); 
	} 
} 
//UDP��������������
void udp_demo_senddata(void)
{
	struct pbuf *ptr;
	ptr = pbuf_alloc(PBUF_TRANSPORT, UDP_DEMO_TX_BUFSIZE, PBUF_POOL); //�����ڴ�
	if(ptr)
	{
		ptr->payload=(void*)udp_demo_sendbuf; 	//���͵�����������
		udp_send(udppcb,ptr);	//udp�������� 
		pbuf_free(ptr);//�ͷ��ڴ�
	} 
} 
//���ڷ�����·ADCת��ֵ(�����ѹ����)
void udp_demo_send_ADCValue(int16_t *value)
{
	int16_t tmp;
	u8 i;
	for (i = 0; i < UDP_DEMO_TX_BUFSIZE / 2; i++)
	{
		tmp = (int16_t)value[i];
		//tmp = (int16_t)(value[i] * 3.3 / 4096 * 800 / 3);
		//���ֽ���ǰ,���ֽ��ں�
		udp_demo_sendbuf[2*i] = (u8)(tmp >> 8);
		udp_demo_sendbuf[2*i + 1] = (u8)tmp;
	}

	udp_demo_senddata(); 
}

/**
 * @brief �ر�UDP����
 * @param upcb UDPЭ����ƿ�
 **/
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);		//�Ͽ�UDP���� 
}
























