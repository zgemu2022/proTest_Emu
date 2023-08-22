#include "protocol_plc.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <arpa/inet.h>
#include "threads_plc.h"
#include "main.h"
int Analysfun06_plc(unsigned char devid, unsigned short reg_addr, unsigned short val)
{
	int sn = reg_addr % 256 - 5;

	int lcdid; //=sn/6;
	int pcsid; //=sn%6;

	lcdid = sn / 6;
	pcsid = sn % 6;
	if (sn >= 0 && sn < 48)
	{
		printf("收到EMU程序发来的功率数据 lcdid=%d pcsid=%d sn=%d\n", lcdid, pcsid, sn);
		return 0;
	}
	{
		return 1;
	}
}

int BakLcdFun03_plc(unsigned char devid, unsigned short reg_addr, unsigned short num_frame)
{
	unsigned char sendbuf[256] = {0};
	int pos = 0;
	unsigned char len = 2;
	unsigned char pdata[] = {0, 0};

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
	if (send(modbus_client_sockptr_plc, sendbuf, pos, 0) < 0)
	{
		printf("返回数据失败！！！！ \n");
		return 0xffff;
	}
	else
	{
		printf("返回数据发送成功！！！！\n");
	}
	return 0;
}
int AnalysModbus_plc(unsigned char *pbuf, int len)
{
	unsigned char plcdata[256];
	unsigned char funid;
	unsigned short regAddr;
	unsigned short num_frame; //[]={0,0,0,0,0,0};
	unsigned char devid;
	num_frame = pbuf[0] * 256 + pbuf[1];
	memcpy(plcdata, &pbuf[6], len - 6);

	devid = plcdata[0];
	funid = plcdata[1];
	regAddr = plcdata[2] * 256 + plcdata[3];

	printf("   funid:%#x    ", funid);

	if (funid == 0x06)
	{
		unsigned short val;
		printf("plc 功能码0x06，regAddr=0x%x len=%d\n", regAddr, len);
		val = plcdata[4] * 256 + plcdata[5];
		Analysfun06_plc(devid, regAddr, val);

		//	myprintbuf(len, pdata);
		if (send(modbus_client_sockptr_plc, pbuf, len, 0) < 0)
		{
			printf("plc 返回数据失败！！！！ \n");
			return 0xffff;
		}
		else
		{

			printf("plc 返回数据发送成功！！！！\n");
		}
	}
	else if (funid == 0x03)
	{

		BakLcdFun03_plc(devid, regAddr, num_frame);
	}
	return 0;
}