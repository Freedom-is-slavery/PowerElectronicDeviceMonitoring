#include "tcp_client_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 	   
 

u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP Client�������ݻ�����
u8 tcp_client_sendbuf[TCP_CLIENT_TX_BUFSIZE];	//TCP Client�������ݻ�����
extern u8 TestStatus;

//TCP Client ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����	!!����ע��!!
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����
u8 tcp_client_flag;	 
struct tcp_pcb *tcppcb;  	//����һ��TCP���������ƿ�

//����Զ��IP��ַ
void tcp_client_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	LCD_ShowString(30,50,200,16,16,"Now TCP Client");
	LCD_ShowString(30,70,200,16,16,"Remote IP Set");  
	LCD_ShowString(30,90,200,16,16,"KEY0:+  KEY2:-");  
	LCD_ShowString(30,110,200,16,16,"KEY_UP:OK");  
	tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
	if(tbuf==NULL)return;
	//ǰ����IP���ֺ�DHCP�õ���IPһ��
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//Զ��IP
	LCD_ShowString(30,150,210,16,16,tbuf); 
	POINT_COLOR=BLUE;
	xoff=strlen((char*)tbuf)*8+30;
	LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0); 
	while(1)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		else if(key)
		{
			if(key==KEY0_PRES)lwipdev.remoteip[3]++;//IP����
			if(key==KEY2_PRES)lwipdev.remoteip[3]--;//IP����
			LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0X80);//��ʾ��IP
		}
	}
	myfree(SRAMIN,tbuf); 
}
 

//TCP Client ����
void tcp_client_test(void)
{
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ

	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
	u8 connflag=0;		//���ӱ��
	
	tcp_client_set_remoteip();//��ѡ��IP
	LCD_Clear(WHITE);	//����
	POINT_COLOR=RED; 	//��ɫ����
	LCD_ShowString(30,50,200,16,16,"Using TCP Client"); 
	LCD_ShowString(30,90,200,16,16,"KEY0:Send data");  
	LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit");  
	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
	LCD_ShowString(30,130,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
	LCD_ShowString(30,150,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote Port:%d",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
	LCD_ShowString(30,170,210,16,16,tbuf);
	POINT_COLOR=BLUE;
	LCD_ShowString(30,190,210,16,16,"STATUS:Disconnected"); 
	
	tcppcb=tcp_new();	//����һ���µ�pcb
	if(tcppcb)			//�����ɹ�
	{
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
 	}
	else 
		res=1;
	
	//TCP_Client ��ѭ��
	while(res==0)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)	
			break;					//KEY_UP����,�˳�TCP_Clientģʽ
		
		if(tcp_client_flag&1<<6)//�Ƿ��յ�����?
		{
			//�������ݲ���,�������ݲ�ʹ��			
			tcp_client_flag&=~(1<<6);//��������Ѿ���������.
		}
		if(tcp_client_flag&1<<5)//�Ƿ�������?
		{
			if(connflag==0)
			{ 
				LCD_ShowString(30,190,lcddev.width-30,lcddev.height-190,16,"STATUS:Connected   ");//��ʾ��Ϣ		
				POINT_COLOR=RED;
				LCD_ShowString(30,210,lcddev.width-30,lcddev.height-190,16,"Receive Data:");//��ʾ��Ϣ		
				POINT_COLOR=BLUE;//��ɫ����
				connflag=1;//���������
			} 
		}else if(connflag)
		{
 			LCD_ShowString(30,190,190,16,16,"STATUS:Disconnected");
			LCD_Fill(30,210,lcddev.width-1,lcddev.height-1,WHITE);//����
			connflag=0;	//������ӶϿ���
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==200)
		{
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//δ������,��������
			{ 
				tcp_client_connection_close(tcppcb,0);//�ر�����
				tcppcb=tcp_new();	//����һ���µ�pcb
				if(tcppcb)			//�����ɹ�
				{ 
					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
				}
			}
			t=0;
			LED0=!LED0;
		}		
	}
	tcp_client_connection_close(tcppcb,0);//�ر�TCP Client����
	myfree(SRAMIN,tbuf);
} 

//lwIP TCP���ӽ�������ûص�����
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //�����ڴ�
		if(es) //�ڴ�����ɹ�
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
			es->pcb=tpcb;  
			es->p=NULL;

			tcp_arg(tpcb,es);        			//ʹ��es����tpcb��callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//��ʼ��LwIP��tcp_recv�ص�����   
			tcp_err(tpcb,tcp_client_error); 	//��ʼ��tcp_err()�ص�����
			tcp_sent(tpcb,tcp_client_sent);		//��ʼ��LwIP��tcp_sent�ص�����
			tcp_poll(tpcb,tcp_client_poll,1); 	//��ʼ��LwIP��tcp_poll�ص����� 
 			tcp_client_flag|=1<<5; 				//������ӵ���������
			TestStatus = STATUS_IS_TCP_CLIENT;	//��ǵ�ǰ���е�TCP_Client,���������Ѿ���
			err=ERR_OK;
		}
		else
		{ 
			tcp_client_connection_close(tpcb,es);//�ر�����
			err=ERR_MEM;	//�����ڴ�������
		}
	}
	else
	{
		tcp_client_connection_close(tpcb,0);//�ر�����
	}
	return err;
}

//lwIP tcp_recv()�����Ļص�����
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len=0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
	if(p==NULL)//����ӷ��������յ��յ�����֡�͹ر�����
	{
		es->state=ES_TCPCLIENT_CLOSING;//��Ҫ�ر�TCP ������ 
 		es->p=p; 
		ret_err=ERR_OK;
	}else if(err!= ERR_OK)//�����յ�һ���ǿյ�����֡,����err!=ERR_OK
	{ 
		if(p)pbuf_free(p);//�ͷŽ���pbuf
		ret_err=err;
	}else if(es->state==ES_TCPCLIENT_CONNECTED)	//����������״̬ʱ
	{
		if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
			}
			tcp_client_flag|=1<<6;		//��ǽ��յ�������
 			tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
		}
	}else  //���յ����ݵ��������Ѿ��ر�,
	{ 
		tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		es->p=NULL;
		pbuf_free(p); //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
}

//lwIP tcp_err�����Ļص�����
void tcp_client_error(void *arg,err_t err)
{  
	//�������ǲ����κδ���
}

//!!���޸�,�������ݵĹ��̲��ܷ���0.5sһ�ε���ѯ��������
//update: ���ھͲ���Ҫ������
//lwIP tcp_poll�Ļص�����
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	return ERR_OK;
} 
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//��������
	return ERR_OK;
}

//�˺���������������
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //��Ҫ���͵����ݼ��뵽���ͻ��������
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//ָ����һ��pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
			pbuf_free(ptr);				//�ͷ�ptr 
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//�����ͻ�������е������������ͳ�ȥ
	} 
}

//ADC�õ�����������ת�������ݷ���
void TCP_Client_send_ADCValue(vu16 *value)
{
	u8 mark;
	struct tcp_client_struct *es; 
	
	for (mark = 0; mark < TCP_CLIENT_TX_BUFSIZE/2; mark++)
	{
		//���ֽ���ǰ,���ֽ��ں�
		tcp_client_sendbuf[2* mark] = (u8)(value[mark] >> 8);
		tcp_client_sendbuf[2* mark + 1] = (u8)value[mark];
	}

	es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
	es->pcb=tcppcb;  
	es->p=NULL;

	if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
	{
		if(tcp_client_flag&(1<<7))	//�ж��Ƿ�������Ҫ���� 
		{
			//���﷢��TCP����,��ַ�ش�С���޸�Ϊ�̶�ֵ(�����ѹ������6��u16����,��ַ�ش�СΪ12)
			es->p=pbuf_alloc(PBUF_TRANSPORT, TCP_CLIENT_TX_BUFSIZE, PBUF_POOL);	//�����ڴ� 
			pbuf_take(es->p,(char*)tcp_client_sendbuf,strlen((char*)tcp_client_sendbuf));	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
			tcp_client_senddata(tcppcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
			tcp_client_flag&=~(1<<7);	//������ݷ��ͱ�־
			if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�
		}
		else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			tcp_client_connection_close(tcppcb,es);//�ر�TCP����
		}
	}
	else
	{ 
		tcp_abort(tcppcb);	//��ֹ����,ɾ��pcb���ƿ�
	}
}

//�ر��������������
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	//�Ƴ��ص�
	tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
	tcp_arg(tpcb,NULL);  
	tcp_recv(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);  
	if(es)mem_free(es); 
	tcp_client_flag&=~(1<<5);//������ӶϿ���
}






















