#ifndef _THREADS_H_
#define _THREADS_H_

#define _SHORT_ 2
#define _BOOL_ 3
#define _UINT_ 4
#define _INT_ 5
#define _U_SHORT 6
#define _FLOAT_ 7

#define ON_LINE 1
#define OFF_LINE 0
// 61850服务器系统状态定义
enum SERVER_WORK_STATE //当前工作状态
{
	SER_WAITTING_START = 0,				  //等待启动中
	SER_IDEL = 1,						  //系统闲置中
	EMS_COMMUNICATION_STATUS_SETTING = 2, //设置EMS通信状态
	EMS_START_EMU = 3,					  //下发升压舱启动信号

	//	SER_IDEL = 0xff, //系统闲置中
};

void CreateThreads_ems(void);
#endif