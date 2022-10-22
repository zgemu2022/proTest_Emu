#include "main.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "threads_bams.h"
#include "threads_ems.h"
#include "threads_plc.h"

PARA_BAMS para_bms = {1, {9600, 9600}, {0, 1}, {6, 14}};
PARA_LCD  para_lcd = {1,{{"192.168.2.230"},{"192.168.2.230"},{"192.168.2.230"},{"192.168.2.230"},{"192.168.2.230"},{"192.168.2.230"},{"192.168.2.230"}},
{1500,503,504,505,506,507}};
int main(int argc, char *argv[])
{
	int i;
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] is %s\n", i, argv[i]);
	// CreateThreads_BAMS((void *)&para_bms);
	// CreateThreads_ems();
	// CreateThreads_PLC();
	CreateThreads_lcd((void*)&para_lcd);
	while (1)
	{
	}
	return 0;
}