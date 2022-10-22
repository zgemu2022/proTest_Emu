#ifndef _MAIN_H_
#define _MAIN_H_

#define PORTNUM_MAX    2
typedef struct
{
	unsigned char portnum;//接入数量
    unsigned short baud[PORTNUM_MAX]; //波特率
	unsigned char devid[PORTNUM_MAX];//设备地址
	unsigned char pcs_num[PORTNUM_MAX];
} PARA_BAMS; //

typedef struct
{
	unsigned char lcdnum;//LCD数量
	char lcd_server_ip[6][64];
	unsigned short server_port[6];

} PARA_LCD;

#endif