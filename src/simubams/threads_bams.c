
#include "threads_bams.h"
#include "main.h"
#include "sys.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "serial.h"

#include "protocol_bams.h"
#include "protocol_lcd.h"
#include "threads_lcd.h"
#include "crc.h"
#include "stdlib.h"
PARA_BAMS *pParaBams;

void Uart_Init(unsigned char portid, unsigned int baud)
{
	int ret;

	printf("正在试图打开串口%d   波特率=%d！！！！！\n", portid, baud);
	ret = OpenComPort(portid, baud, 8, "1", 'N');
	if (ret == -1)
		printf("串口%d打开失败！！！！！\n", portid);
	else
	{
		printf("串口%d打开成功  波特率=%d！！！！！\n", portid, baud);
	}
}

void *serial_thread(void *arg)
{

	int portid = (int)arg;
	int pcsid = 0;
	int num_send = 0;
	int res;
	printf("serial_thread 端口号 =%d pcs数量=%d\n", portid, pParaBams->pcs_num[portid]);
	if (pParaBams->pcs_num[portid] == 0)
	{
		while (1)
		{
			sleep(1);
		}
	}

	Uart_Init(portid, pParaBams->baud[portid]);

	while (1)
	{

		res = doFunTasks(portid, &pcsid);
		if (res == 0)
		{
			printf("收到返回数据！！！！！portid=%d \n", portid);
		}
		else
			printf("未能收到返回数据！！！！！res=%d portid=%d\n", res, portid);
		// sleep(1);
		printf("当前BAMS 端口portid=%d pcsid=%d\n", portid, pcsid);
		usleep(200000); // 延时500ms
	}
}

void CreateThreads_BAMS(void *para)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;
	int i;

	pParaBams = (PARA_BAMS *)para;

	// for (i = 0; i < 3; i++)
	// {
	// 	if (modbus_sockt_state_set[i] != 0)
	// 		pParaBams->pcs_num[0] += yx1246[i];
	// }

	// for (i = 3; i < 6; i++)
	// {
	// 	if (modbus_sockt_state_set[i] != 0)
	// 		pParaBams->pcs_num[1] += yx1246[i];
	// }

	printf("BAMS 的个数 =%d 每个BAMS中pcs数量 %d %d \n", pParaBams->portnum, pParaBams->pcs_num[0], pParaBams->pcs_num[1]);
	for (i = 0; i < pParaBams->portnum; i++)
	{

		if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)serial_thread, (int *)i, 1, 1))
		{
			printf("MODBUS CONNECT THTREAD CREATE ERR!\n");

			exit(1);
		}
	}

	printf("MODBUS THTREAD CREATE success!\n");
}