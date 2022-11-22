#include "main.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "threads_bams.h"
#include "threads_ems.h"
#include "threads_plc.h"

char modbus_sockt_state_set[] = {1, 1, 1, 1, 1, 1};
unsigned short yx1246[] = {0x6, 0x5, 0x4, 0x5, 0x5, 0x4};
int g_maxid_pcs=0;
int g_flag_RecvNeed_LCD = 0;

unsigned char flag_RecvNeed_PCS[MAX_PCS_NUM];
unsigned int countRecvFlag(int num_read)
{
	unsigned int flag = 0;
	int i;
	for (i = 0; i < num_read; i++)
	{
		flag |= 1 << i;
	}
	return flag;
}
int myprintbuf(int len, unsigned char *buf)
{
	int i = 0;
	printf("\nbuflen=%d\n", len);
	for (i = 0; i < len; i++)
		printf("0x%x ", buf[i]);
	printf("\n");
	return 0;
}
PARA_BAMS para_bms = {2, {9600, 9600}, {0, 1}, {6, 5}};

PARA_LCD para_lcd = {6, {{"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}}, {1502, 1503, 1504, 1505, 1506, 1507}};
int main(int argc, char *argv[])
{
	int i,j=0;
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] is %s\n", i, argv[i]);
    for(i=6;i>0;i--)
	{
		if(modbus_sockt_state_set[i-1]==1)
		{
			if(j==0)
			   j=i;
			g_flag_RecvNeed_LCD |= (1<<(i-1));
			flag_RecvNeed_PCS[i-1] = countRecvFlag(yx1246[i-1]);
		}

	}

	if(j==0)
	{
		printf("没有有效的pcs存在\n");
	}
	else 
	{
		g_maxid_pcs = (j-1)*6+yx1246[j-1];
	}

	printf("最大的pcs编号 g_maxid_pcs=%d g_flag_RecvNeed_LCD=%x  \n",g_maxid_pcs,g_flag_RecvNeed_LCD);
	printf("flag_RecvNeed_PCS %x  %x  %x  %x  %x %x\n",flag_RecvNeed_PCS[0],flag_RecvNeed_PCS[1],flag_RecvNeed_PCS[2], flag_RecvNeed_PCS[3],flag_RecvNeed_PCS[4],flag_RecvNeed_PCS[5]);

    // CreateThreads_ems();
	// CreateThreads_PLC();
	//CreateThreads_BAMS((void *)&para_bms);
    CreateThreads_ems();
	//  CreateThreads_PLC();
	CreateThreads_lcd((void *)&para_lcd);
	CreateThreads_BAMS((void *)&para_bms);
    CreateThreads_plc();
	while (1)
	{
	}
	return 0;
}