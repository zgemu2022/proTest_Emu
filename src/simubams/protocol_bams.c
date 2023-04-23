#include "protocol_bams.h"
#include "sys.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "serial.h"
#include <string.h>

#include "crc.h"
#include "stdlib.h"
#include "threads_bams.h"
#include "main.h"
int g_bms_status = BMS_ST_INIT;
// short para_soc[] = {111, 322, 533, 744, 855, 966};
// short para_soc[] = {400, 400, 400, 400, 400, 400,400, 400, 400, 400, 400, 400,400, 400, 400, 400, 400, 400};
static int createFunFrame(int portid, int *pPcsid, int *pNumsend, int *pLenframe, unsigned char *framebuf)
{
	int pcsid = *pPcsid;
	int numsend = *pNumsend;
	int numTask = ARRAY_LEN(bamsfun)/2;
	printf("numTask:%d \n",numTask);

	printf("PROTEST_EMU createFunFrame:portid=%d pcsid=%d numsend=%d Bams'sNum=%d\n", portid, pcsid, numsend, pParaBams->pcs_num[portid]);
	unsigned short regstart = bamsfun[0][0].RegStart + 16 * pcsid;
	int pos = 0; //, pos1;
	int len = 0;
	int i = 0,j=0;
	int startPcs,poscurpcs;
	unsigned short crcval;

	framebuf[pos++] = pParaBams->devid[portid];
	framebuf[pos++] = 0x10;
	framebuf[pos++] = regstart / 256;
	framebuf[pos++] = regstart % 256;
	framebuf[pos++] = (numTask*16) / 256;
	framebuf[pos++] = (numTask*16) % 256;
	len = numTask*2*16;
	// pos1 = pos;
	framebuf[pos++] = len;
	if(pcsid == 7){
		startPcs = 7;
		poscurpcs = 14;
	}else{
		startPcs=0;
		poscurpcs =7;
	}
	for (i = startPcs; i < poscurpcs ; i++)
	{
		for(j=0;j<16;j++){
			framebuf[pos++] = bamsfun[i][j].para / 256;
			framebuf[pos++] = bamsfun[i][j].para % 256;
		}
		pcsid++;
	}

	crcval = crc16_check(&framebuf[0], pos);
	framebuf[pos++] = crcval / 256;
	framebuf[pos++] = crcval % 256;
	numsend++;
	if (numsend == MAX_SEND_NUM)
	{
		if (pcsid == (pParaBams->pcs_num[portid]))
		{
			pcsid = 0;
		}
		numsend = 0;
	}
	*pLenframe = pos;
	*pPcsid = pcsid;
	*pNumsend = numsend;
	return 0;
}

int doFunTasks(int portid, int *pPcsid, int *pNumsned)
{
	unsigned char commbuf[256];
	bzero(commbuf, sizeof(commbuf));

	int lencomm = 0, lentemp, i;

	unsigned short crcval;
	unsigned char b1, b2;

	createFunFrame(portid, pPcsid, pNumsned, &lencomm, commbuf);
	printf("端口portid=%d 准备发送数据包长度为:%d  内容为：", portid, lencomm);
	for (i = 0; i < lencomm; i++)
	{
		// printf("%#x ", commbuf[i]);
		 printf("%02x ", commbuf[i]);
	}
	printf("\n");
	int res = WriteComPort(portid, commbuf, lencomm);
	if (res > 0)
	{
		printf(" 端口portid=%d 发送成功 res=%d\n", portid, res);
	}
	else
	{
		printf("端口portid=%d 发送失败 ret=%d\n", portid, res);
		return 4;
	}
	lentemp = ReadComPort(portid, commbuf, 256);

	if (lentemp == 0)
	{
		printf("端口portid=%d lentemp:%d\n", portid, lentemp);
		return 253;
	}
	else if (lentemp == -1)
	{
		return 255;
	}

	crcval = crc16_check(commbuf, lentemp - 2);

	b1 = crcval / 256;
	b2 = crcval % 256;
	// printf("lentemp长度：%d  b1:%#x  b2:%#x  commbuf[lentemp-2]:%#x commbuf[lentemp-1]:%#x\n", lentemp, b1, b2, commbuf[8 - 2], commbuf[8 - 1]);
	if (b1 != commbuf[lentemp - 2] && b2 != commbuf[lentemp - 1])
		return 254;

	if (commbuf[1] == 0x90)
		return commbuf[2];

	printf("模拟BAMS收到正确返回:%d\n", lentemp);
	return 0;
}