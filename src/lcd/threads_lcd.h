#ifndef _THREADS_LCD_H_
#define _THREADS_LCD_H_
#define MAX_MODBUS_FLAME 1024

typedef struct
 {
	  int msgtype;
	  char data[MAX_MODBUS_FLAME];
}msgClient;
typedef struct
{
	unsigned char  buf[MAX_MODBUS_FLAME];	//数据
	int len;	//buf的长度
}MyData;

typedef struct
{
	unsigned short frameId;//帧序号
	unsigned char recvbuf[MAX_MODBUS_FLAME];//收到的数据部分
	unsigned short lenrecv;//收到的数据长度

}ModTcp_Frame;
void CreateThreads_lcd(void* para);
#endif