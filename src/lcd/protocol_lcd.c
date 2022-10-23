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
#include "protocol_lcd.h"
int BakLcdFun10(int id_thread, unsigned short reg_addr, unsigned short num, unsigned short num_frame)
{
	// printf(" id_thread:%d  reg_addr:%#x  val:%#x\n",id_thread,reg_addr,val);

	// printf("ssssssss\n");
	unsigned char sendbuf[256];
	int pos = 0;
	sendbuf[pos++] = num_frame / 256;
	sendbuf[pos++] = num_frame % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = 0x0a;
	sendbuf[pos++] = 0x10;
	sendbuf[pos++] = reg_addr / 256;
	sendbuf[pos++] = reg_addr % 256;
	sendbuf[pos++] = num / 256;
	sendbuf[pos++] = num % 256;
	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("返回数据失败！！！！ id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{

		printf("返回数据发送成功！！！！\n");
	}
}
int AnalysModbus(int id_thread, unsigned char *pdata, int len)
{
	unsigned char emudata[256];
	unsigned char funid;
	unsigned short regAddr;
	unsigned short num_frame; //[]={0,0,0,0,0,0};

	// num_frame[id_thread]=pdata[0] * 256 + pdata[1]
	num_frame = pdata[0] * 256 + pdata[1];

	memcpy(emudata, &pdata[6], len - 6);
	funid = emudata[1];
	regAddr = emudata[2] * 256 + emudata[3];
	printf("id_thread:%d   ", id_thread);
	printf("   funid:%#x    ", funid);

	if (funid == 0x10)
	{
		int num = emudata[4] * 256 + emudata[5];
		BakLcdFun10(id_thread, regAddr, num, num_frame);
		if (regAddr == 0x3050) //功能码0x10，设置时间
			printf("功能码0x10，emu设置时间！！！！num=%d\n", num);
	}
	// else if (funid == 0x06)
	// {
	// 	if (send(modbus_client_sockptr[id_thread], pdata, len, 0) < 0)
	// 	{
	// 		printf("返回数据失败！！！！ id_thread=%d\n", id_thread);
	// 		return 0xffff;
	// 	}
	// 	else
	// 	{

	// 		printf("返回数据发送成功！！！！\n");
	// 	}
	// }

	return 0;
}