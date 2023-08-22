
#ifndef _PROTOCOL_BAMS_H_
#define _PROTOCOL_BAMS_H_

#define BMS_MX_CPW 0						 // 电池分系统 n 最大允许充电功率
#define BMS_MX_DPW 1						 // 电池分系统 n 最大允许放电功率
#define BMS_CONN_HEARTBEAT 2				 // 电池分系统 n 通讯心跳
#define BMS_MAIN_VOLTAGE 3					 // 电池分系统 n 总电压
#define BMS_MX_CCURRENT 4					 // 电池分系统 n 最大允许充电电流
#define BMS_MX_DCURRENT 5					 // 电池分系统 n 最大允许放电电流
#define BMS_SUM_CURRENT 6					 // 电池分系统 n 电池总电流
#define BMS_SOC 7							 // 电池分系统 n 电池 SOC
#define BMS_remaining_charging_capacity 8	 // 电池分系统 n 电池剩余可充电量
#define BMS_remaining_discharging_capacity 9 // 电池分系统 n 电池剩余可放电量
#define BMS_single_MX_voltage 10			 // 电池分系统 n 单体最高电压
#define BMS_single_MI_voltage 11			 // 电池分系统 n 单体最低电压
#define BMS_SYS_STATUS 12					 // 电池分系统 n 状态0-初始化 1-停机 2-启动中 3-运行 4-待机 5-故障 9-关机 255-调试
#define BMS_SYS_NEED 13						 // 电池分系统 n 需求 0-禁充禁放(PCS禁止充电放电, PCS应停机或封脉冲) 1-只能充电（PCS禁止放电） 2-只能放电（PCS禁止充电） 3-可充可放（正常）
#define BMS_FAULT_STATUS 14					 // 电池分系统 n 总故障状态
#define MAX_SEND_NUM 1
enum _BMS_SYS_STATUS // 电池分系统状态
{
	BMS_ST_INIT = 0,	 //-初始化
	BMS_ST_PAUSE = 1,	 //-停机
	BMS_ST_START = 2,	 //-启动中
	BMS_ST_WORKING = 3,	 //-运行
	BMS_ST_WAITTING = 4, //-待机
	BMS_ST_FAULT = 5,	 //-故障
	BMS_ST_SHUTDOWN = 9, //-关机
	BMS_ST_TEST = 255	 //-调试
};
typedef struct
{
	unsigned char funid;	 // 功能码
	unsigned short RegStart; // 寄存器开始地址
	short para;				 // 设置参数
				//	unsigned short numData;//数据个数
} BAMS_Fun_Struct;

extern BAMS_Fun_Struct bamsfun[];
// extern BAMS_Fun_Struct bamsfun[14][16];
extern int g_bms_status;
int doFunTasks(int portid, int *pPcsid);
#endif
