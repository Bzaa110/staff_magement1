 /*******************************************************
 # File Name: client.c
 # Author: Bzaa
 # Mail: bzaa1321@163.com
 # Created  Time:2020年12月06日 星期日 16时35分27秒
 # Function:
 *******************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define N 32

#define REGISTER    1 //user - register
#define LOGIN       2 //user - login 
#define ADD_IFO     3 //user - add_info 
#define MODIFY_IFO  4 //user - modify_info 
#define DELETE_IFO  5 //user - delete_info 
#define QUERY_IFO   6 //user - query_info 

//定义通信双方的信息结构体
typedef struct
{
	int type;
	char name[N];
	char data[256];
}MSG;

int do_register(int sockfd, MSG *msg)
{
	msg->type = REGISTER;
	printf("请输入名字:");
	scanf("%s", msg->name);
	getchar();//干掉垃圾字符

	printf("请输入密码:");
	scanf("%s", msg->data);

	if(send(sockfd, (void *)msg, sizeof(MSG), 0) < 0)
	{
		printf("fail to send\n");	
		return -1;
	}

	if(recv(sockfd, (void *)msg, sizeof(MSG), 0) < 0)
	{
		printf("fail to receive\n");	
		return -1;
	}

	//注册成功 or 该姓名已经注册
	printf("%s\n", msg->data);

	return 0;

}

int do_login(int sockfd, MSG *msg)
{
	msg->type = LOGIN;
	printf("请输入名字:");
	scanf("%s", msg->name);
	getchar();//干掉垃圾字符

	printf("请输入密码:");
	scanf("%s", msg->data);

	if(send(sockfd, (void *)msg, sizeof(MSG), 0) < 0)
	{
		printf("fail to send\n");	
		return -1;
	}

	if(recv(sockfd, (void *)msg, sizeof(MSG), 0) < 0)
	{
		printf("fail to receive\n");	
		return -1;
	}
	if(strncmp(msg->data, "登录成功", 3) == 0)
	{
		printf("登录成功！\n");
		return 1;
	}
	else
	{
		printf("%s\n", msg->data);
	}
	return 0;
}
int do_add_info(int sockfd, MSG *msg)
{
	printf("新增员工信息...\n");

	return 1;
}
int do_modify_info(int sockfd, MSG *msg)
{
	printf("修改员工信息...\n");

	return 1;
}
int do_delete_info(int sockfd, MSG *msg)
{
	printf("删除员工信息...\n");

	return 1;
}
int do_query_info(int sockfd, MSG *msg)
{
	printf("查询员工信息...\n");

	return 0;
}

// ./client 192.168.3.196 10000
int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	int n;
	MSG msg;


	if(argc != 3)
	{
		printf("Usage:%s serverip port!\n", argv[0]);
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket.\n");
		return -1;
	}

	/*------------填充地址信息结构体-------------*/
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0 )
	{
		perror("fail to connect");
		return -1;
	}
	while(1)
	{
		printf("***************员工管理系统V1.0版******************\n");
		printf("*                                                 *\n");
		printf("*                  登录界面                       *\n");
		printf("*-------------------------------------------------*\n");
		printf("*                                                 *\n");
		printf("* 1.注册账户       2.登录账户         3.退出      *\n");
		printf("*                                                 *\n");
		printf("*                                                 *\n");
		printf("***************************************************\n");
		printf("请选择:");
		scanf("%d", &n);
		getchar(); //去掉垃圾字符

		switch(n)
		{
			case 1:
				do_register(sockfd, &msg);
				break;
			case 2:
				if(do_login(sockfd, &msg) == 1)
				{
					goto next;
				}
				break;
			case 3:
				close(sockfd);
				exit(0);
				break;
			default:
				printf("Invalid data cmd. \n");
		}

	}
next:
	while(1)
	{
		
		printf("***************员工管理系统V1.0版******************\n");
		printf("*                                                 *\n");
		printf("*                  用户界面                       *\n");
		printf("*-------------------------------------------------*\n");
		printf("*                                                 *\n");
		printf("* 1.新增员工信息  2.修改员工信息 3.删除员工信息   *\n");
		printf("* 4.查询员工信息  5.退 出                         *\n");
		printf("*                                                 *\n");
		printf("***************************************************\n");
		printf("请选择:");
		scanf("%d", &n);
		getchar(); //回收垃圾字符
		switch(n)
		{
			case 1:
				do_add_info(sockfd, &msg);
				break;
			case 2:
				do_modify_info(sockfd, &msg);
				break;
			case 3:
				do_delete_info(sockfd, &msg);
				break;
			case 4:
				do_query_info(sockfd, &msg);
				break;
			case 5:
				close(sockfd);
				exit(0);
				break;

			default:
				printf("Invalid data cmd. \n");
		}
	}




    return 0;
}
