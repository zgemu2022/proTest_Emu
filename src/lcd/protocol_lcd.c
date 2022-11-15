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
#include "YC_Define.h"
#include "YX_Define.h"
#include <math.h>
#include "threads_ems.h"
unsigned char yxData_ZJ_1240[] = {0x0, 0x43, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x6};																						  //整机遥信数据数据长度=14,数据个数=7
unsigned char yxData_MK[] = {0x0, 0x41, 0x50, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x80, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x80, 0x1};			  //模块遥信数据
unsigned char ycData_ZJ_1174[] = {0x0, 0x1, 0x0, 0x3, 0x0, 0x0, 0x1b, 0x4e, 0x1b, 0x25, 0x1b, 0xe, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x0, 0x0, 0x13, 0x88, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x25, 0x4e}; //整机遥测数据数据长度=30
unsigned char ycData_MK[] = {0x0, 0x4, 0x0, 0x1, 0x0, 0x0, 0x1b, 0x85, 0x1b, 0x91, 0x1b, 0x87, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x13, 0x88, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
							 0x5, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}; //模块遥测数据，数据长度58 数据个数29

unsigned short val_qw[6][6];
unsigned short val_pw[6][6];
unsigned short cmd_start_pcs[6][6];
unsigned short status_pcs_lcd[6][6];
int check_task_finished(int lcdid, int pcsid, int taskid)
{
	static int taskid_last[] = {0,0};
	static unsigned char flag_recv_pcs[2][6] = {{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0}};
	static int flag_recv_lcd[] = {0,0};
	int i;
		printf("aaa taskid=%d check_task_finished lcdid=%d  pcsid=%d lcd=%x  pcs=%x %x %x %x %x %x \n",taskid,lcdid, pcsid,g_flag_RecvNeed_LCD,flag_RecvNeed_PCS[0],flag_RecvNeed_PCS[1],flag_RecvNeed_PCS[2],
	flag_RecvNeed_PCS[3],flag_RecvNeed_PCS[4],flag_RecvNeed_PCS[5]);
	if (taskid_last[taskid-1] != taskid)
	{
		flag_recv_lcd[taskid-1] = 0;
		for (i = 0; i < 6; i++)
			flag_recv_pcs[taskid-1][i] = 0;

		flag_recv_pcs[taskid-1][lcdid] |= (1 << (pcsid - 1));
		taskid_last[taskid-1] = taskid;
		return 0;

	}
	flag_recv_pcs[taskid-1][lcdid] |= (1 << (pcsid - 1));
	if (flag_recv_pcs[taskid-1][lcdid] == flag_RecvNeed_PCS[lcdid])
	{

		flag_recv_lcd[taskid-1] |= (1 << lcdid);
	}
	if (flag_recv_lcd[taskid-1] == g_flag_RecvNeed_LCD)
	{
		printf("111check_task_finished lcd=%x  pcs=%x %x %x %x %x %x\n",flag_recv_lcd[taskid-1],flag_recv_pcs[taskid-1][0],flag_recv_pcs[taskid-1][1],flag_recv_pcs[taskid-1][2],
	    flag_recv_pcs[taskid-1][3],flag_recv_pcs[taskid-1][4],flag_recv_pcs[taskid-1][5]);
		taskid_last[taskid-1] = 0;
		return taskid;
	}

	printf("000check_task_finished lcd=%x pcs=%x %x %x %x %x %x\n",flag_recv_lcd[taskid-1],flag_recv_pcs[taskid-1][0],flag_recv_pcs[taskid-1][1],flag_recv_pcs[taskid-1][2],
	flag_recv_pcs[taskid-1][3],flag_recv_pcs[taskid-1][4],flag_recv_pcs[taskid-1][5]);
	return 0;

}

int checkOn_off_finish(int lcdid, int pcsid, int flag)
{
	static unsigned char flag_recv_pcs[] = {0, 0, 0, 0, 0, 0};
	int i;
	static int flag_recv_lcd = 0;
	static int flag_on_off_last = 0;

	if (flag_on_off_last != flag)
	{
		flag_recv_lcd = 0;
		for (i = 0; i < 6; i++)
			flag_recv_pcs[i] = 0;
		flag_recv_pcs[lcdid] |= (1 << (pcsid - 1));

		flag_on_off_last = flag;
		return 0;
	}

	flag_recv_pcs[lcdid] |= (1 << (pcsid - 1));
	if (flag_recv_pcs[lcdid] == flag_RecvNeed_PCS[lcdid])
	{

		flag_recv_lcd |= (1 << lcdid);
	}
	if (flag_recv_lcd == g_flag_RecvNeed_LCD)
	{
		return flag;
	}
	return 0;
}
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
	case 0x3000:
	{
		pcsid = 1;
	}
	break;
	case 0x3018:
	case 0x3015:
	case 0x3012:
	case 0x1210:
	case 0x111D:
	case 0x3010:
		pcsid = 2;
		break;
	case 0x3028:
	case 0x3025:
	case 0x3022:
	case 0x1220:
	case 0x113A:
	case 0x3020:
		pcsid = 3;
		break;
	case 0x3038:
	case 0x3035:
	case 0x3032:
	case 0x1230:
	case 0x1157:
	case 0x3030:
		pcsid = 4;
		break;
	case 0x3068:
	case 0x3065:
	case 0x3062:
	case 0x1250:
	case 0x1190:
	case 0x3060:
		pcsid = 5;
		break;
	case 0x3078:
	case 0x3075:
	case 0x3072:
	case 0x1260:
	case 0x11AD:
	case 0x3070:
		pcsid = 6;
		break;
	default:
		break;
	}
	return pcsid;
}
short Apparent_Power(int pw, int qw)
{
	short aw = 0;
	int temp;
	pw *= pw;
	qw *= qw;
	temp = pw + qw;

	aw = (short)(sqrt(temp));
	return aw;
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
		pdata = &data_buf[0];
		pdata[0] = yx1246[id_thread] / 256;
		pdata[1] = yx1246[id_thread] % 256;

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
		if (cmd_start_pcs[id_thread][pcsid - 1] == 0xff00)
		{
			pdata[u16_InvRunState1 * 2] = 0x40;
			pdata[u16_InvRunState1 * 2 + 1] = 0x00;
			printf("启动情况下的遥信数据\n");
			myprintbuf(30, pdata);
		}
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
		short aw;
		if (cmd_start_pcs[id_thread][pcsid - 1] == 0xff00)
		{
			pdata[Active_power * 2] = val_pw[id_thread][pcsid - 1] / 256;
			pdata[Active_power * 2 + 1] = val_pw[id_thread][pcsid - 1] % 256;
			pdata[Reactive_power * 2] = val_qw[id_thread][pcsid - 1] / 256;
			pdata[Reactive_power * 2 + 1] = val_qw[id_thread][pcsid - 1] % 256;
			aw = Apparent_Power(val_pw[id_thread][pcsid - 1], val_qw[id_thread][pcsid - 1]);
			pdata[Apparent_power * 2] = aw / 256;
			pdata[Apparent_power * 2 + 1] = aw % 256;
		}

		len = 58;
		printf("模块遥测功能码0x03，lcd[%d]，模块编号pcsid=%d 遥测数据长度=58 val=%d\n", id_thread, pcsid, val_qw[id_thread][pcsid - 1]);
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

int BakLcdFun10(int id_thread, unsigned char devid, unsigned short reg_addr, unsigned short num, unsigned short num_frame)
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
		val_pw[id_thread][pcsid - 1] = val * 10;
	    printf("在PQ并且恒功率模式下设置有功功率，设置lcdid=%d 模块编号pcsid=%d 为val=%d\n", id_thread, pcsid, val);
			if (check_task_finished(id_thread, pcsid, ADJUST_PW) == ADJUST_PW)
			{
				printf("LCD[%d]全部pcs收到有功功率调节指令 pcsid=%d val=%d\n", id_thread,pcsid,val);

			}

	}
	else if (reg_addr == 0x3002 || reg_addr == 0x3012 || reg_addr == 0x3022 || reg_addr == 0x3032 || reg_addr == 0x3062 || reg_addr == 0x3072)
	{
		pcsid = getPcsid(reg_addr);
		val_qw[id_thread][pcsid - 1] = val * 10;
		printf("在PQ并且恒功率模式下设置无功功率，设置lcdid=%d 模块编号pcsid=%d 为val=%d\n", id_thread, pcsid, val);
			if (check_task_finished(id_thread, pcsid, ADJUST_QW) == ADJUST_QW)
			{
				printf("LCD[%d]全部pcs收到无功功率调节指令 pcsid=%d val=%d\n", id_thread,pcsid,val);
				if (g_sys_status_last == ADJUST_EMU_QW && g_sys_status == SER_IDEL)
				{
					printf("设置EMU停机操作\n");
					g_sys_status = EMS_STOP_EMU; 
					g_sys_status_last = ADJUST_EMU_QW;
				}


			}

	}
	else if (reg_addr == 0x3000 || reg_addr == 0x3010 || reg_addr == 0x3020 || reg_addr == 0x3030 || reg_addr == 0x3060 || reg_addr == 0x3070)
	{
		pcsid = getPcsid(reg_addr);
		if (val == 0xff00)
		{

			printf("LCD[%d]PCS[%d]收到开机指令\n", id_thread, pcsid);
			if (checkOn_off_finish(id_thread, pcsid, 0x55) == 0x55)
			{
				printf("LCD[%d]全部pcs收到开机指令\n", id_thread);
				if (g_sys_status_last == EMS_START_EMU && g_sys_status == SER_IDEL)
				{
					printf("设置LCD[%d]进行无功功率调节\n", id_thread);
					g_sys_status = ADJUST_EMU_QW; //状态进入设置EMS通信状态设置
					g_sys_status_last = EMS_START_EMU;
				}
			}
		}
		else if (val == 0x00ff)
		{
			printf("LCD[%d]PCS[%d]收到关机指令\n", id_thread, pcsid);
			if (checkOn_off_finish(id_thread, pcsid, 0xaa) == 0xaa)
			{
				printf("LCD[%d]全部pcs收到关机指令\n", id_thread);
			}
		}
		else
			printf("收到开关机指令，但数据错误！！！\n");
		cmd_start_pcs[id_thread][pcsid - 1] = val;
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
	printf("uuuuuuuuuuuuuuuuu id_thread=%d  len=%d\n", id_thread, len);
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

void initData(void)
{
	int i, j;
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 6; j++)
		{
			val_qw[i][j] = 0;
			val_pw[i][j] = 0;
			status_pcs_lcd[i][j] = 0;
			cmd_start_pcs[i][j] = 0x00ff;
		}
	}
}