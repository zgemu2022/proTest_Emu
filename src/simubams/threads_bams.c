
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
	int pcsid;
	pcsid = 0;
	int res;
	printf("端口号 =%d \"n", portid);
	Uart_Init(portid, pParaBams->baud[portid]);
	while (1)
	{
		res = doFunTasks(portid, &pcsid);
		if (res == 0)
		{
			printf("收到返回数据！！！！！\n");
		}
		else
			printf("未能收到返回数据！！！！！res=%d\n", res);
		sleep(1);
		// usleep(300000); //延时500ms
	}
}

void CreateThreads_BAMS(void *para)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;
	int i;
	pParaBams = (PARA_BAMS *)para;
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