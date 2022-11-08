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
unsigned short yx1246[] = {0x6, 0x5, 0x4, 0x5, 0x5,0x4};
unsigned char yxData_ZJ_1240[] = {0x0, 0x43, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x6};																						  //整机遥信数据数据长度=14,数据个数=7
unsigned char yxData_MK[] = {0x0, 0x41, 0x50, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x80, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x80, 0x1};			  //模块遥信数据
unsigned char ycData_ZJ_1174[] = {0x0, 0x1, 0x0, 0x3, 0x0, 0x0, 0x1b, 0x4e, 0x1b, 0x25, 0x1b, 0xe, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x0, 0x0, 0x13, 0x88, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x25, 0x4e}; //整机遥测数据数据长度=30
unsigned char ycData_MK[] = {0x0, 0x4, 0x0, 0x1, 0x0, 0x0, 0x1b, 0x85, 0x1b, 0x91, 0x1b, 0x87, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x13, 0x88, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
							 0x5, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}; //模块遥测数据，数据长度58 数据个数29

unsigned char getPcsid(unsigned short reg_addr)
{
	unsigned char pcsid = 0;
	switch (reg_addr)
	{
	case 0x3008:
	case 0x3005:	
	case 0x3002:
	case 0x1200:		
	case 0x1100:			
	{
		pcsid = 1;
	}
	break;
	case 0x3018:
	case 0x3015:
	case 0x3012:
	case 0x1210:
	case 0x111D:
		pcsid = 2;
		break;
	case 0x3028:
	case 0x3025:
	case 0x3022:
	case 0x1220:
	case 0x113A:
		pcsid = 3;
		break;
	case 0x3038:
	case 0x3035:
	case 0x3032:
	case 0x1230:
	case 0x1157:
		pcsid = 4;
		break;
	case 0x3068:
	case 0x3065:
	case 0x3062:
	case 0x1250:
	case 0x1190:
		pcsid = 5;
		break;
	case 0x3078:
	case 0x3075:
	case 0x3072:
	case 0x1260:
	case 0x11AD:
		pcsid = 6;
		break;
	default:
		break;
	}
	return pcsid;
}

int BakLcdFun03(int id_thread, unsigned char devid, unsigned short reg_addr, unsigned short num_frame)
{
	unsigned char sendbuf[256] = {0};
	int pos = 0;
	unsigned char len = 0;
	unsigned char *pdata;
	unsigned char pcsid;
    unsigned char data_buf[4];

	if (reg_addr == 0x1246)
	{
        pdata=&data_buf[0];
		pdata[0] = yx1246[id_thread]/256;
		pdata[1] = yx1246[id_thread]%256;

		len = 2;
	}
	else if (reg_addr == 0x1240)
	{
		printf("整机遥信功能码0x03，lcd[%d]整机遥信，数据长度=14 \n", id_thread);
		pdata = yxData_ZJ_1240;
		len = 14;
	}
	else if (reg_addr == 0x1200 || reg_addr == 0x1210 || reg_addr == 0x1220 || reg_addr == 0x1230 || reg_addr == 0x1250 || reg_addr == 0x1260)
	{
		pcsid = getPcsid(reg_addr);
		pdata = yxData_MK;
		len = 30;
		printf("模块遥信功能码0x03，lcd[%d]，模块编号pcsid=%d 遥信数据长度=30 \n", id_thread, pcsid);
	}
	else if (reg_addr == 0x1174)
	{
		printf("整机遥测功能码0x03，lcd[%d]整机遥测，数据长度=30 \n", id_thread);
		pdata = ycData_ZJ_1174;
		len = 30;
	}
	else if (reg_addr == 0x1100 || reg_addr == 0x111D || reg_addr == 0x113A || reg_addr == 0x1157 || reg_addr == 0x1190 || reg_addr == 0x11AD)
	{
		pcsid = getPcsid(reg_addr);
		pdata = ycData_MK;
		len = 58;
		printf("模块遥测功能码0x03，lcd[%d]，模块编号pcsid=%d 遥测数据长度=58 \n", id_thread, pcsid);

	}

	sendbuf[pos++] = num_frame / 256;
	sendbuf[pos++] = num_frame % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = devid;
	sendbuf[pos++] = 0x03;
	sendbuf[pos++] = len;
	memcpy(&sendbuf[pos], pdata, len);
	pos += len;
	sendbuf[5] = pos - 6;
	myprintbuf(pos, sendbuf);
	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("返回数据失败！！！！ id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{

		printf("返回数据发送成功！！！！\n");
	}
	return 0;
}

int BakLcdFun10(int id_thread,unsigned char devid, unsigned short reg_addr, unsigned short num, unsigned short num_frame)
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
	sendbuf[pos++] = devid;
	sendbuf[pos++] = 0x10;
	sendbuf[pos++] = reg_addr / 256;
	sendbuf[pos++] = reg_addr % 256;
	sendbuf[pos++] = num / 256;
	sendbuf[pos++] = num % 256;
	sendbuf[5] = pos - 6;
	myprintbuf(pos, sendbuf);
	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("返回数据失败！！！！ id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{

		printf("返回数据发送成功！！！！\n");
	}
	return 0;
}

// 0x3046	产品运行模式设置	uint16	整机	1	5	"需在启机前设置，模块运行后无法进行设置
// 1：PQ模式（高低穿功能，需选择1）；
// 5：VSG模式（并离网功能，需选择5）；"
int Analysfun06(int id_thread, unsigned char devid, unsigned short reg_addr, unsigned short val)
{
	unsigned char pcsid;

	if (reg_addr == 0x3046)
	{
		printf("整机产品运行模式设置 lcdid=%d val=%d(1：PQ模式 5：VSG模式)\n", id_thread, val);
	}
	else if (reg_addr == 0x3008 || reg_addr == 0x3018 || reg_addr == 0x3028 || reg_addr == 0x3038 || reg_addr == 0x3068 || reg_addr == 0x3078)
	{
		pcsid = getPcsid(reg_addr);
		printf("在PQ模式下，设置lcdid=%d pcsid=%d 为val=%d(0：恒功率模式 3：恒流模式)\n", id_thread, pcsid, val);
	}
	else if (reg_addr == 0x3005 || reg_addr == 0x3015 || reg_addr == 0x3025 || reg_addr == 0x3035 || reg_addr == 0x3065 || reg_addr == 0x3075)
	{
		pcsid = getPcsid(reg_addr);
		printf("在PQ并且恒功率模式下设置有功功率，设置lcdid=%d 模块编号pcsid=%d 为val=%d\n", id_thread, pcsid, val);
	}
	else if (reg_addr == 0x3002 || reg_addr == 0x3012 || reg_addr == 0x3022 || reg_addr == 0x3032 || reg_addr == 0x3062 || reg_addr == 0x3072)
	{
		pcsid = getPcsid(reg_addr);
		printf("在PQ并且恒功率模式下设置无功功率，设置lcdid=%d 模块编号pcsid=%d 为val=%d\n", id_thread, pcsid, val);
	}
	else
		printf("uuuyyyrrrxxxxxxxxxxxxxxxxxxxxxxreg_addr=%x \n", reg_addr);
	return 0;
}

int AnalysModbus(int id_thread, unsigned char *pdata, int len)
{
	unsigned char emudata[256];
	unsigned char funid;
	unsigned short regAddr;
	unsigned short num_frame; //[]={0,0,0,0,0,0};
	unsigned char devid;
	// num_frame[id_thread]=pdata[0] * 256 + pdata[1]
	num_frame = pdata[0] * 256 + pdata[1];
    printf("uuuuuuuuuuuuuuuuu id_thread=%d  len=%d\n",id_thread,len);
	memcpy(emudata, &pdata[6], len - 6);
	devid = emudata[0];
	funid = emudata[1];
	regAddr = emudata[2] * 256 + emudata[3];
	printf("id_thread:%d   ", id_thread);
	printf("   funid:%#x    ", funid);

	if (funid == 0x10)
	{
		int num = emudata[4] * 256 + emudata[5];
		BakLcdFun10(id_thread, devid, regAddr, num, num_frame);
		if (regAddr == 0x3050) //功能码0x10，设置时间
			printf("功能码0x10，emu设置时间！！！！num=%d\n", num);
	}
	else if (funid == 0x06)
	{
		unsigned short val;
		printf("功能码0x06，regAddr=0x%x\n", regAddr);
		val = emudata[4] * 256 + emudata[5];
		Analysfun06(id_thread, devid, regAddr, val);

	//	myprintbuf(len, pdata);
		if (send(modbus_client_sockptr[id_thread], pdata, len, 0) < 0)
		{
			printf("返回数据失败！！！！ id_thread=%d\n", id_thread);
			return 0xffff;
		}
		else
		{

			printf("返回数据发送成功！！！！\n");
		}
	}
	else if (funid == 0x03)
	{
		BakLcdFun03(id_thread, devid, regAddr, num_frame);
	}
	return 0;
}