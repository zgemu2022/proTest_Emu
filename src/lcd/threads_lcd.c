#include "threads_lcd.h"
#include "my_socket.h"
#include "sys.h"
#include "main.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/msg.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_CLIENT_CNT 10
#define STATUS_ON 1
#define STATUS_OFF 0
char modbus_sockt_state[MAX_CLIENT_CNT];
unsigned int modbus_sockt_timer[MAX_CLIENT_CNT];
int modbus_client_sockptr[MAX_CLIENT_CNT];
struct sockaddr_in Server_Addr, Client_addr;
PARA_LCD *pParaLcd;
int g_comm_qmegid[6];
int wait_flag = 0;
unsigned short g_num_frame = 1;
static int myprintbuf(int len, unsigned char *buf)
{
	int i = 0;
	printf("\nbuflen=%d\n", len);
	for (i = 0; i < len; i++)
		printf("0x%x ", buf[i]);
	printf("\n");
	return 0;
}
void *Modbus_clientSend_thread(void *arg) // 25
{

	int id_thread = (int)arg;

	int ret_value = 0;
	msgClient pmsg;
	MyData pcsdata;
	int waittime = 0;
	int id_frame;

	printf("PCS[%d] Modbus_clientSend_thread  is Starting!\n", id_thread);

	wait_flag = 0;

	// printf("modbus_sockt_state[id_thread] == STATUS_ON\n") ;
	while (modbus_sockt_state[id_thread] == STATUS_ON) //
	{
		// printf("wait_flag:%d\n", wait_flag);
		ret_value = os_rev_msgqueue(g_comm_qmegid[id_thread], &pmsg, sizeof(msgClient), 0, 100);
		if (ret_value >= 0)
		{
			waittime = 0;
			memcpy((char *)&pcsdata, pmsg.data, sizeof(MyData));

			id_frame = pcsdata.buf[0] * 256 + pcsdata.buf[1];

			if ((id_frame != 0xffff && (g_num_frame - 1) == id_frame) || (id_frame == 0xffff && g_num_frame == 1))
			{
				printf("recv form pcs!!!!!g_num_frame=%d  id_frame=%d\n", g_num_frame, id_frame);
			//	int res = AnalysModbus(id_thread, pcsdata.buf, pcsdata.len);
				// if (0 == res)
				// {
				// 	printf("数据解析成功！！！\n");
				// }
			}
			else
				printf("检查是否发生丢包现象！！！！！g_num_frame=%d  id_frame=%d\n", g_num_frame, id_frame);
			wait_flag = 0;
			continue;
		}

		if (wait_flag == 1)
		{
			waittime++;
			if (waittime == 1000)
			{
				wait_flag = 0;
				waittime = 0;
			}
			continue;
		}

//		RunAccordingtoStatus(id_thread);
	}
	return NULL;
}
static int recvFrame(int fd, int qid, MyData *recvbuf)
{
	int readlen;

	// int index = 0, length = 0;
	//  int i;
	msgClient msg;
	// MyData *precv = (MyData *)&msg.data;
	readlen = recv(fd, recvbuf->buf, MAX_MODBUS_FLAME, 0);
	//		readlen = recv(fd, (recvbuf.buf + recvbuf.len),
	//				(MAX_BUF_SIZE - recvbuf.len), 0);
	//		printf("*****  F:%s L:%d recv readlen=%d\n", __FUNCTION__, __LINE__,	readlen);
	if (readlen < 0)
	{
		printf("连接断开或异常\r\n");
		return -1;
	}
	else if (readlen == 0)
		return 1;

	printf("收到一包数据 wait_flag=%d", wait_flag);
	recvbuf->len = readlen;
	myprintbuf(readlen, recvbuf->buf);
	msg.msgtype = 1;
	memcpy((char *)&msg.data, recvbuf->buf, readlen);
	sleep(1);
	if (msgsnd(qid, &msg, sizeof(msgClient), IPC_NOWAIT) != -1)
	{

		printf("succ succ succ succ !!!!!!!"); //连接主站的网络参数I
		return 0;
	}
	else
	{
		return 1;
	}

	// for(i=0;i<readlen;i++)
	// 	printf("0x%2x ",recvbuf->buf[i]);
	// printf("\n");
}

void *Modbus_clientRecv_thread(void *arg) // 25
{
	int id_thread = (int)arg;
	int fd = -1;
	fd_set maxFd;
	struct timeval tv;
	int ret;
	int i = 0, jj = 0;
	MyData recvbuf;
	printf("lcd[%d] Modbus_clientRecv_thread is Starting!\n", id_thread);

	while (1)
	{
		fd = modbus_client_sockptr[id_thread];
		if (fd == -1)
			break;
		FD_ZERO(&maxFd);
		FD_SET(fd, &maxFd);
		tv.tv_sec = 0;
		//    tv.tv_usec = 50000;
		tv.tv_usec = 200000;
		ret = select(fd + 1, &maxFd, NULL, NULL, &tv);
		if (ret < 0)
		{

			printf("网络有问题！！！！！！！！！！！！");
			break;
		}
		else if (ret == 0)
		{
			jj++;

			if (jj > 1000)
			{
				printf("暂时没有数据传入！！！！未接收到数据次数=%d！！！！！！！！！！！！！！！！\r\n", jj);
				jj = 0;

				//				break;
			}
			continue;
		}
		else
		{

			jj = 0;

			// printf("貌似收到数据！！！！！！！！！！！！");
			if (FD_ISSET(fd, &maxFd))
			{
				ret = recvFrame(fd, g_comm_qmegid[id_thread], &recvbuf);
				if (ret == -1)
				{
					i++;

					if (i > 30)
					{
						printf("接收不成功！！！！！！！！！！！！！！！！i=%d\r\n", i);
						break;
					}
					else
						continue;
				}
				else if (ret == 1)
				{
					//                 i++;

					// if(i>30)
					// {
					// 	printf("接收数据长度为0！！！！！！！！！！！！！！！！\r\n");

					// 	i=0;

					// }
					continue;
				}
				else
				{
					i = 0;
					printf("接收成功！！！！！！！！！！！！！！！！wait_flag=%d modbus_sockt_state[id_thread]=%d\r\n", wait_flag, modbus_sockt_state[id_thread]);
				}
			}
			else
			{
				printf("未知错误////////////////////////////////r/n");
				break;
			}
		}
	}
	modbus_sockt_state[id_thread] = STATUS_OFF;
	close(modbus_client_sockptr[id_thread]);
	printf("网络断开，退出现场，等待重连！！！！");
	return NULL;
}



void *Modbus_ServerConnectThread(void *arg)
{
	int id_thread = (int)arg;
	int client_sockptr;
	pthread_t ThreadID;
	int ret;
	pthread_attr_t Thread_attr;
	_SERVER_SOCKET server_sock;

	key_t key = 0;
	g_comm_qmegid[id_thread] = os_create_msgqueue(&key, 1);
	printf("ip=%s  port=%d\n",pParaLcd->lcd_server_ip[id_thread],pParaLcd->server_port[id_thread]);

	server_sock.protocol = TCP;
	server_sock.port = htons(pParaLcd->server_port[id_thread]);
	server_sock.addr = inet_addr(pParaLcd->lcd_server_ip[id_thread]);
	server_sock.fd = -1;
	ret = _socket_server_init(&server_sock);
	if(ret<0)
	{
            printf("_socket_server_init err \n");
			while(1)
			{

			}
	}
		
	printf("_socket_server_init succ\n");

	while (1)
	{
		if (modbus_sockt_state[id_thread] == STATUS_OFF)
		{
			client_sockptr = _socket_server_listen(server_sock);
			if (client_sockptr > 0)
			{
				 modbus_sockt_state[id_thread] = STATUS_ON;
				 modbus_client_sockptr[id_thread]= client_sockptr;
				if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientRecv_thread, (int *)id_thread, 1, 1))
				{
					printf("MODBUS CONNECT THTREAD CREATE ERR!\n");
				}
				// if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientSend_thread, (int *)id_thread, 1, 1))
				// {
				// 	printf("MODBUS THTREAD CREATE ERR!\n");
				// }
			}
			else 
			  printf("err err 999999999999 client_sockptr=%d\n",client_sockptr);
		}
		sleep(1);
	}

	return NULL;
}

void CreateThreads_lcd(void *para)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;
	pParaLcd = (PARA_LCD *)para;
	int i;

	for (i = 0; i < pParaLcd->lcdnum; i++)
	{
		modbus_sockt_state[i] = STATUS_OFF;
		if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_ServerConnectThread, (int *)i, 1, 1))
		{
			printf("Modbus_ServerConnectThread CREATE ERR\n");

			exit(1);
		}
	}

	printf("12345 ssdlh\n");
}