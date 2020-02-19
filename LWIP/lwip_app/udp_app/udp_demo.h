#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
//////////////////////////////////////////////////////////////////////////////////	 
//
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define UDP_DEMO_RX_BUFSIZE		100     //����udp���������ݳ���
#define UDP_DEMO_TX_BUFSIZE     12       //����udp�������ݳ���(�˴�����ADת��ͨ��������Ҫ) 
#define UDP_DEMO_PORT			8089	//����udp���ӵĶ˿� 
 
void udp_demo_test(void);
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void udp_demo_senddata(void);
void udp_demo_connection_close(struct udp_pcb *upcb);

void udp_demo_send_ADCValue(vu16 *value);

#endif

