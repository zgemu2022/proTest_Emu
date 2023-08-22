#ifndef _MAIN_H_
#define _MAIN_H_

#define PORTNUM_MAX 4
#define MAX_PCS_NUM 6 // 每个LCD下最多包含pcs的个数
#define MAX_LCD_NUM 6

#define STATUS_ON 1
#define STATUS_OFF 0
typedef struct
{
	unsigned char portnum;			  // 接入数量
	unsigned short baud[PORTNUM_MAX]; // 波特率
	unsigned char devid[PORTNUM_MAX]; // 设备地址
	unsigned char pcs_num[PORTNUM_MAX];
} PARA_BAMS; //

typedef struct
{
	unsigned char lcdnum; // LCD数量
	char lcd_server_ip[6][64];
	unsigned short server_port[6];

} PARA_LCD;

extern char modbus_sockt_state_set[];
extern unsigned short yx1246[];
extern int g_maxid_pcs;
extern int g_flag_RecvNeed_LCD;

extern unsigned char flag_RecvNeed_PCS[];

int myprintbuf(int len, unsigned char *buf);
#endif