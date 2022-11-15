#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <stdlib.h>
#include <bits/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "IEC61850_type.h"
#include "threads_ems.h"
#include "command.h"
#include "sys.h"

sem_t *sem1 = NULL;
sem_t *sem2 = NULL;
sem_t *sem3 = NULL;

sem_t *mutex1_lock = NULL;
sem_t *mutex2_lock = NULL;

int g_sys_status = SER_WAITTING_START;
int g_sys_status_last = SER_WAITTING_START;

static iec61850_shm_packet_t *shm_addr;
int total_pcsnum=0;
static void emu_commnication_status_setting(void)
{

	int i;

	i = shm_addr->shm_que2.wpos;
	shm_addr->shm_que2.wpos++;
	shm_addr->shm_que2.slist[i].sAddr.portID = 1;
	shm_addr->shm_que2.slist[i].sAddr.devID = 1;
	shm_addr->shm_que2.slist[i].sAddr.typeID = 9;
	shm_addr->shm_que2.slist[i].sAddr.pointID = 0;

	shm_addr->shm_que2.slist[i].data_size = 1;
	shm_addr->shm_que2.slist[i].data[0] = 1;

	printf("emu_commnication_status_setting发送的数据  %d  \n", (int)shm_addr->shm_que2.slist[i].data[0]);
	shm_addr->shm_que2.slist[i].el_tag = _BOOL_;
}

static void ems_start_emu(unsigned char type)
{

	int i;

	i = shm_addr->shm_que2.wpos;
	shm_addr->shm_que2.wpos++;
	shm_addr->shm_que2.slist[i].sAddr.portID = 1;
	shm_addr->shm_que2.slist[i].sAddr.devID = 1;
	shm_addr->shm_que2.slist[i].sAddr.typeID = 5;
	if (type == 0)
		shm_addr->shm_que2.slist[i].sAddr.pointID = 2;
	else
		shm_addr->shm_que2.slist[i].sAddr.pointID = 1;

	shm_addr->shm_que2.slist[i].data_size = 1;
	shm_addr->shm_que2.slist[i].data[0] = type;

	printf("emus_start_emu发送的数据  %d  \n", (int)shm_addr->shm_que2.slist[i].data[0]);
	shm_addr->shm_que2.slist[i].el_tag = _BOOL_;
}

static void ems_start_stop_onepcs(unsigned char type, unsigned char lcdid, unsigned char pcsid)
{

	int i;
	unsigned char sn;
	sn = lcdid * 6 + pcsid;
	i = shm_addr->shm_que2.wpos;
	shm_addr->shm_que2.wpos++;
	shm_addr->shm_que2.slist[i].sAddr.portID = 3;
	shm_addr->shm_que2.slist[i].sAddr.devID = sn;
	shm_addr->shm_que2.slist[i].sAddr.typeID = 5;
	shm_addr->shm_que2.slist[i].sAddr.pointID = 1;

	shm_addr->shm_que2.slist[i].data_size = 1;
	shm_addr->shm_que2.slist[i].data[0] = type;

	printf("ems_start_onepcs发送的数据  %d  \n", (int)shm_addr->shm_que2.slist[i].data[0]);
	shm_addr->shm_que2.slist[i].el_tag = _BOOL_;
}
static void ems_adjust_pw_qw(unsigned char type,float val)
{

	int i;

	i = shm_addr->shm_que2.wpos;
	shm_addr->shm_que2.wpos++;
	shm_addr->shm_que2.slist[i].sAddr.portID = 1;
	shm_addr->shm_que2.slist[i].sAddr.devID = 1;
	shm_addr->shm_que2.slist[i].sAddr.typeID = 6;
    shm_addr->shm_que2.slist[i].sAddr.pointID = type;
	shm_addr->shm_que2.slist[i].data_size = 4;
	shm_addr->shm_que2.slist[i].el_tag = _FLOAT_;
    *(float*)shm_addr->shm_que2.slist[i].data=val;

}

void bms_setting(int sys_status)
{

	switch (sys_status)
	{
	case EMS_COMMUNICATION_STATUS_SETTING:
		emu_commnication_status_setting();
		break;
	case EMS_START_EMU:
		ems_start_emu(1);
		break;
	case EMS_STOP_EMU:
		ems_start_emu(0);
		break;
	case EMS_START_ONE_PCS:
		ems_start_stop_onepcs(1, 0, 3);
		break;
	case EMS_STOP_ONE_PCS:
		ems_start_stop_onepcs(0, 0, 3);
		break;
	case ADJUST_EMU_QW:
	    ems_adjust_pw_qw(2,14*total_pcsnum);
		break;
	default:
		break;
	}
}

void *thread_61850_write(void *arg)
{
	while (g_sys_status == SER_WAITTING_START)
		sleep(1);

	while (1)
	{
		if (g_sys_status <= SER_IDEL)
		{
			if (g_sys_status == SER_IDEL)
			{
				sleep(1);
				printf("12345 ssdlh\n");
			}

			else
				usleep(20000);
			continue;
		}
		else
		{
			printf("55555 55555  g_sys_status=%d %d\n", g_sys_status, EMS_COMMUNICATION_STATUS_SETTING);
			//	sleep(1);
		}

		if (g_sys_status > SER_IDEL)
		{
			printf("66666 666666  g_sys_status=%d\n", g_sys_status);
			sem_wait(mutex2_lock);
			printf("77777 777777  g_sys_status=%d\n", g_sys_status);
			bms_setting(g_sys_status);

			sem_post(mutex2_lock);

			sem_post(sem2);
			g_sys_status_last = g_sys_status;
			g_sys_status = SER_IDEL;
		}
	}

	return NULL;
}

int anslize()
{
	int i = 0;
	data_info_t temp_data;

	sem_wait(mutex1_lock);
	while (shm_addr->shm_que1.wpos != shm_addr->shm_que1.rpos)
	{
		i = shm_addr->shm_que1.rpos;
		printf("rpos=%d  数据标识 %d %d %d %d", i, shm_addr->shm_que1.slist[i].sAddr.portID, shm_addr->shm_que1.slist[i].sAddr.devID, shm_addr->shm_que1.slist[i].sAddr.typeID, shm_addr->shm_que1.slist[i].sAddr.pointID);
		data_info_t temp_data = shm_addr->shm_que1.slist[i];
		if (temp_data.sAddr.portID == 1 && temp_data.sAddr.devID == 1 && temp_data.sAddr.typeID == 2 && temp_data.sAddr.pointID == 0)
		{

			total_pcsnum = *(int *)temp_data.data;

			printf("获得emu管理的全部pcs数量 %d！\n",total_pcsnum);

			// if(g_sys_status == SER_WAITTING_START)
			g_sys_status = EMS_COMMUNICATION_STATUS_SETTING; //状态进入设置EMS通信状态设置
			g_sys_status_last = SER_WAITTING_START;
			printf("状态进入设置EMS通信状态设置\n");
			
		}
		else if (temp_data.sAddr.portID == 1 && temp_data.sAddr.devID == 1 && temp_data.sAddr.typeID == 2 && temp_data.sAddr.pointID == 19)
		{
			printf("共享内存收到遥测统计数据，系统状态进入启动！\n");
			if (g_sys_status_last == EMS_COMMUNICATION_STATUS_SETTING && g_sys_status == SER_IDEL)
			{
				g_sys_status = EMS_START_EMU; // EMS_START_ONE_PCS;
			}
		}

		switch (temp_data.el_tag)
		{
		case _INT_:
		{
			int tem;
			tem = *(int *)temp_data.data;
			printf("收到整形数=%d\n", tem);
		}

		break;
		case _FLOAT_:
		{
			float tem;
			tem = *(float *)temp_data.data;
			printf("收到浮点数=%f\n", tem);
		}

		break;
		case _BOOL_:
		{
			printf("收到BOOL数据 %x\n", temp_data.data[0]);
		}
		break;
		case _U_SHORT:
		{
			unsigned short tem;
			tem = *(unsigned short *)temp_data.data;
			printf("收到ushort数据 %x\n", tem);
		}
		break;

		default:
			break;
		}

		shm_addr->shm_que1.rpos++;
		shm_addr->shm_que1.rpos %= 1024;
	}
	sem_post(mutex1_lock);
	return 0;
}
void *thread_61850_read(void *arg)
{
	printf("333thread_61850_read\n");

	int id = create_ipc(sizeof(iec61850_shm_packet_t));
	int i1 = 0;
	struct timespec waittime;
	struct timeval nowtime;
	iec61850_shm_packet_t *p;
	if (id < 0)
	{
		printf("create sharememory error\n");
		return 0;
	}
	id = 0;

	// if ((sem1 = sem_open(SHM_SEM1_PATH, O_RDWR, 0644, 0)) == SEM_FAILED)
	if ((sem1 = sem_open(SHM_SEM1_PATH, O_CREAT | O_RDWR, 0644, 1)) == SEM_FAILED)
	{
		puts("<111>.semaphore is not exit");
		usleep(1);
		//	umask(mask);

		return 0;
	}
	else
		puts("<111>.semaphore sem1 open succ");
	mutex1_lock = sem_open(SHM_MUTEX1_PATH, O_CREAT | O_RDWR, 0644, 1);
	if (mutex1_lock == SEM_FAILED)
	{
		puts("<2>.semaphore is not exit");
		usleep(1);
		return 0;
	}

	if ((sem2 = sem_open(SHM_SEM2_PATH, O_CREAT | O_RDWR, 0644, 1)) == SEM_FAILED)
	{
		puts("<3>.semaphore is not exit");
		usleep(1);
		return 0;
	}
	else
		puts("<3>.semaphore sem2 open succ");

	if ((mutex2_lock = sem_open(SHM_MUTEX2_PATH, O_CREAT | O_RDWR, 0644, 1)) == SEM_FAILED)
	{
		puts("<4>.semaphore is not exit");
		usleep(1);
		return 0;
	}

	id = get_ipc(sizeof(iec61850_shm_packet_t));
	if (id < 0)
	{
		printf("get sharememory error\n");
		goto end;
	}
	p = (iec61850_shm_packet_t *)shmat(id, NULL, 0);
	if (p < 0)
	{
		printf("get sharememory addr error\n");
		p = NULL;
		goto end;
	}

	shm_addr = p;
	//	CreateThreads()	;
	while (1)
	{

		gettimeofday(&nowtime, NULL);
		waittime.tv_sec = nowtime.tv_sec + 1;
		waittime.tv_nsec = 0;

		if (sem_timedwait(sem1, &waittime))
		{
			perror("test -- 1  <12>.semaphore\n");
			continue;
		}
		else
		{
			printf("收到数据！！！！wpos=%d rpos=%d\n", p->shm_que1.wpos, p->shm_que1.rpos);

			anslize();
			continue;
		}

		printf("stest61850 is runing i=%d\n", i1++);
	}

end:
	if (id != 0)
	{
		if (destroy_sharememory(id) < 0)
		{
			printf("destroy error\n");
		}
	}

	return NULL;
}

void CreateThreads_ems(void)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;

	if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)thread_61850_read, NULL, 1, 1))
	{
		printf(" thread_61850_read CREATE ERR!\n");
		exit(1);
	}
	if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)thread_61850_write, NULL, 1, 1))
	{
		printf(" thread_61850_write CREATE ERR!\n");
		exit(1);
	}

	printf("thread_61850 CREATE success!\n");
}