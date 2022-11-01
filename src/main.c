#include "main.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "threads_bams.h"
#include "threads_ems.h"
#include "threads_plc.h"
int myprintbuf(int len, unsigned char *buf)
{
	int i = 0;
	printf("\nbuflen=%d\n", len);
	for (i = 0; i < len; i++)
		printf("0x%x ", buf[i]);
	printf("\n");
	return 0;
}
PARA_BAMS para_bms = {1, {9600, 9600}, {0, 1}, {6, 14}};

PARA_LCD para_lcd = {6, {{"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}, {"192.168.2.230"}}, {1502, 1503, 1504, 1505, 1506, 1507}};
int main(int argc, char *argv[])
{
	int i;
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] is %s\n", i, argv[i]);
	// CreateThreads_BAMS((void *)&para_bms);
	// CreateThreads_ems();
	// CreateThreads_PLC();
	// CreateThreads_BAMS((void *)&para_bms);
	//  CreateThreads_ems();
	//  CreateThreads_PLC();
	CreateThreads_lcd((void *)&para_lcd);
	while (1)
	{
	}
	return 0;
}