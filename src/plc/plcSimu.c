#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <pthread.h>
#include "sys.h"
#include "threads_plc.h"

#define MAXBUF 1024
typedef struct{
    char server_ip[64];
    int port;
}CONF;

CONF server_conf = {{"192.168.3.230"},502};
int waitFlag;



void *serial_thread_plc(void)
{
    printf("PLC 测试线程 启动 \n");
    int sockfd, new_fd,i;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    waitFlag = 0;
    
    unsigned int myport, lisnum;
    char buf[MAXBUF + 1];
    
    fd_set rfds;
    struct timeval tv;
    
    int retval, maxfd = -1;

 
	lisnum = 10;

    // create an new socket
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("PLC socket");
        exit(1);
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(server_conf.port);
    my_addr.sin_addr.s_addr = inet_addr(server_conf.server_ip);

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) 
    {
        perror("PLC bind");
        exit(1);
    }

    if (listen(sockfd, lisnum) == -1) 
    {
        perror("listen");
        exit(1);
    }
    printf("\nPLC 服务器已打开监听...  ip:%s,port:%d\n", server_conf.server_ip, server_conf.port);
    while (1) 
    {
        len = sizeof(struct sockaddr);
	
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)) == -1) 
	{
            perror("PLC accept");
            exit(errno);
        } 
        else
	{
            printf("PLC server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
	}
	
        // 开始处理每个新连接上的数据收发
	
        while (1) 
	{
            // 把集合清空 
            FD_ZERO(&rfds);
	    
            // 把标准输入(stdin)句柄0加入到集合中
            FD_SET(0, &rfds);
    
            // 把当前连接(socket)句柄new_fd加入到集合中 
            FD_SET(new_fd, &rfds);
	    
	    maxfd = 0;
            if (new_fd > maxfd)
	    {
                maxfd = new_fd;
	    }
	    
            // 设置最大等待时间 
            tv.tv_sec = 5;
            tv.tv_usec = 0;
	    
            // 开始等待 
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
	    
            if (retval == -1) 
	    {
                printf("PLC 将退出，select出错！ %s", strerror(errno));
                break;
            } 
            else if (retval == 0) 
	    {
                printf("PLC 没有任何消息到来，继续等待……\n");
                continue;
            } 
            else 
	    {
            // 判断当前IO是否是来自socket
            if (FD_ISSET(new_fd, &rfds)) // 当前连接的socket上有消息到来则接收对方发过来的消息并显示
            {
                bzero(buf, MAXBUF + 1);
                // 接收客户端的消息
                len = recv(new_fd, buf, MAXBUF, 0);
                if (len > 0)
                {
                    printf("PLC  接收到的数据为：");
                    for(i=0;i<len;i++){
                        printf("%#x ",buf[i]);
                    }
                    printf("\n");
                    
                    if(buf[7] == 0x06){
                        len = send(new_fd, buf, len, 0);
                    }else if(0x03){
                        unsigned char sendData = 10;
                        buf[5] = 5;
                        buf[8] = 2;
                        buf[9] = sendData/256;
                        buf[10] = sendData%256;
                        len = send(new_fd, buf, 11, 0);
                    }
                    if(len>0){
                        printf("PLC 发送的数据为：");
                        for (i = 0; i < len; i++)
                        {
                            printf("%#x ", buf[i]);
                        }
                        printf("\n");
                    }
                }
                else
                {
                    if (len < 0)
                        printf("PLC  消息接收失败！错误代码是%d，错误信息是'%s'\n", errno, strerror(errno));
                    else
                        printf("PLC 对方退出了，聊天终止\n");
						close(new_fd);
                    break;
                }
            }
        }
                

                    }
                }
        
       
        // 处理每个新连接上的数据收发结束
    close(sockfd);
}

void CreateThreads_PLC(void)
{
    printf("进入PLC 测试");
    pthread_t ThreadID;
    pthread_attr_t Thread_attr;
    int i;
    for (i = 0; i < 1; i++)
    {
        if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)serial_thread_plc, NULL, 1, 1))
        {
            printf("PLC CONNECT THTREAD CREATE ERR!\n");

            exit(1);
        }
    }

    printf("MODBUS THTREAD CREATE success!\n");
}