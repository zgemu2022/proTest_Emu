
#include "threads.h"
#include "sys.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "serial.h"
#include "main.h"
#include "protocol_bams.h"
#include "crc.h"
#include "stdlib.h"
PARA_BAMS *pParaBams;

BAMS_Fun_Struct bamsfun[] = {
	{0x10, 0x0001, 1500}, //电池分系统 n 最大允许充电功率
						  // PCS 直流侧充电功率不得超出该值，PCS 计算上报的可用交流充电功率时，可按照该值上报给EMS或上位机。
						  // 举例：BAMS 报给 PCS 的电池分系统最大允许充电功率为 1500kW，则 PCS 上报给 EMS 的分系统最大允
						  // 许充电功率为 1500kW
	{0x10, 0x0002, 1470}, //电池分系统 n 最大允许放电功率
						  // //PCS 直流侧放电功率不得超出该值，PCS 计算上报的可用交流放电功率时，可按照该值乘以PCS 放电效率
						  // //后上报给 EMS 或上位机。
						  // //举例：BAMS 报给 PCS 的电池分系统最大允许放电功率为 1500kW，PCS 放电效率为 98%，则 PCS上报给
						  // // EMS 的分系统最大允许放电功率为1500×98%=1470kW
	{0x10, 0x0003, 0},	  //电池分系统 n 通讯心跳
						  // //0~128 累加，PCS 判断心跳值不更新超过 10s，认为通讯中断，PCS 应该主动停机

	{0x10, 0x0004, 2000}, //电池分系统 n 总电压

	{0x10, 0x0005, 10}, //电池分系统 n 最大允许充电电流
						// //优先按照最大允许充电功率运行，在不满足以最大允许充电功率运行条件下，可按照最大允许充电电流判
						// //断，PCS 直流侧充电电流不得超过该值，PCS计算上报的可用交流充电电流时，可按照该值上报给 EMS 或
						// //上位机。
	{0x10, 0x0006, 10}, //电池分系统 n 最大允许放电电流
						// //优先按照最大允许放电功率运行，在不满足以最大允许放电功率运行条件下，可按照最大允许放电电流判
						// //断，PCS 直流侧放电电流不得超过该值，PCS 计算上报的可用交流放电电流时，可按照该值乘以 PCS 放电
						// //效率后上报给 EMS或上位机。

	{0x10, 0x0007, 8},	 //电池分系统 n 电池总电流(有符号整型)
						 // //负值代表对电池充电，正值代表对电池放电，PCS 根据 BAMS 报送的当前电流值，与自身采集电流值比较，
						 // //若电流差超过 10A，应告警提示
	{0x10, 0x0008, 50},	 //电池分系统 n 电池 SOC:0%~100%，正常运行区间 5%-95%
	{0x10, 0x0009, 200}, //电池分系统 n 电池剩余可充电量(kWh)

	{0x10, 0x000a, 200}, //电池分系统 n 电池剩余可放电量(kWh)
	{0x10, 0x000b, 300}, //电池分系统 n 单体最高电压
						 // //单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最高电压达到3.6V，PCS 应停机
						 // //或封脉冲；电池分系统单体最高电压达到 3.63V，PCS 应关机；

	{0x10, 0x000c, 300}, //电池分系统 n 单体最低电压
						 // //单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最低电压达到2.85V，PCS 应停
						 // //机或封脉冲；电池分系统单体最低电压达到 2.75V，PCS 应关机；

	{0x10, 0x000d, 3}, //电池分系统 n 状态
					   // //0-初始化 1-停机 2-启动中 3-运行 4-待机 5-故障 9-关机 255-调试

	{0x10, 0x000e, 3}, //电池分系统 n 需求
					   // //0-禁充禁放(PCS禁止充电放电, PCS应停机或封脉冲)
					   // // 1-只能充电（PCS禁止放电）
					   // // 2-只能放电（PCS禁止充电）
					   // // 3-可充可放（正常）
	{0x10, 0x000f, 0}, //电池分系统 n 总故障状态
					   // //0-正常 1-故障，故障时，PCS 应停机，封脉冲

};

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

static int createFunFrame(int portid, int *pPcsid, int *pLenframe, unsigned char *framebuf)
{
	int pcsid = *pPcsid;
	int numTask = ARRAY_LEN(bamsfun);
	unsigned short regstart = bamsfun[0].RegStart + 16 * pcsid;
	int pos = 0; //, pos1;
	int len = 0;
	int i = 0;
	unsigned short crcval;

	framebuf[pos++] = pParaBams->devid[portid];
	framebuf[pos++] = bamsfun[0].funid;
	framebuf[pos++] = regstart / 256;
	framebuf[pos++] = regstart % 256;
	framebuf[pos++] = numTask / 256;
	framebuf[pos++] = numTask % 256;
	len = numTask * 2;
	// pos1 = pos;
	framebuf[pos++] = len;

	for (i = 0; i < numTask; i++)
	{
		framebuf[pos++] = bamsfun[i].para / 256;
		framebuf[pos++] = bamsfun[i].para % 256;
	}

	crcval = crc16_check(&framebuf[0], pos);
	framebuf[pos++] = crcval / 256;
	framebuf[pos++] = crcval % 256;

	pcsid++;
	if (pcsid == (pParaBams->pcs_num[portid]))
	{
		pcsid = 0;
	}
	*pLenframe = pos;
	*pPcsid = pcsid;
	return 0;
}

static int doFunTasks(int portid, int *pPcsid)
{
	unsigned char commbuf[256];
	bzero(commbuf, sizeof(commbuf));

	int lencomm = 0, lentemp, i;

	unsigned short crcval;
	unsigned char b1, b2;

	createFunFrame(portid, pPcsid, &lencomm, commbuf);
	printf("准备发送数据包长度为:%d  内容为：", lencomm);
	for (i = 0; i < lencomm; i++)
	{
		printf("%#x ", commbuf[i]);
	}
	printf("\n");
	int res = WriteComPort(portid, commbuf, lencomm);
	if (res > 0)
	{
		printf(" 发送成功 res=%d\n", res);
	}
	else
	{
		printf("发送失败 ret=%d\n", res);
		return 4;
	}
	lentemp = ReadComPort(portid, commbuf, 256);
	printf("lentemp:%d\n", lentemp);

	if (lentemp == 0)
	{
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