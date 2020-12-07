 /*******************************************************
 # File Name: server.c
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
#include <unistd.h>
#include <sqlite3.h>
#include <signal.h>
#include <string.h>


#define N 32

#define REGISTER    1 //user - register
#define LOGIN       2 //user - login 
#define ADD_IFO     3 //user - add_info 
#define MODIFY_IFO  4 //user - modify_info 
#define DELETE_IFO  5 //user - delete_info 
#define QUERY_IFO   6 //user - query_info 

#define DATABASE "my.db"

//定义通信双方的信息结构体
typedef struct
{
	int type;
	char name[N];
	char data[256];
}MSG;

int do_client(int acceptfd, sqlite3 *db);
void do_register(int acceptfd, MSG *msg, sqlite3 *db);
int do_login(int acceptfd, MSG *msg, sqlite3 *db);
int do_add_info(int acceptfd, MSG *msg, sqlite3 *db);
int do_modify_info(int acceptfd, MSG *msg, sqlite3 *db);
int do_delete_info(int acceptfd, MSG *msg, sqlite3 *db);
int do_query_info(int acceptfd, MSG *msg, sqlite3 *db);

// ./server 192.168.177.128 10000
int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	int n;
	MSG msg;
	sqlite3 *db = NULL;//定义数据库句柄指针
	int acceptfd;
	pid_t pid;

	
	//打开数据库
	if(sqlite3_open(DATABASE, &db) != SQLITE_OK)
	{
		printf("%s\n", sqlite3_errmsg(db));
		return -1;
	}
	else
	{
		printf("打开数据库成功\n");
	}

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


	/*-将保存在相应地址信息结构体中的信息与套接字进行绑定-*/
	if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)//
	{
		perror("fail to bind\n");
		return -1;
	}


	/*------------将套接字设为监听模式-----------*/
	if(listen(sockfd, 5) < 0)
	{
		perror("fail to liesten\n");
		return -1;
	}

	//处理僵尸进程
	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		if((acceptfd = accept(sockfd, NULL, NULL)) < 0)
		{
			perror("fail to accept");
			return -1;
		}

		/*------让父进程接收请求，子进程处理具体的消息------------*/
		if((pid = fork()) < 0)
		{
			perror("fail to fork");
			return -1;
		}
		else if(pid == 0)
		{
			//子进程用来处理客户端具体的信息
			close(sockfd);
			do_client(acceptfd, db);
		}
		else
		{
			//父进程用来接收客户端的请求
			close(acceptfd);
		}

	}

    return 0;
}

int do_client(int acceptfd, sqlite3 *db)
{
	MSG msg;
	while(recv(acceptfd, &msg, sizeof(msg), 0) > 0)
	{
		switch(msg.type)
		{
			printf("type:%d\n", msg.type);
			case REGISTER:
				do_register(acceptfd, &msg, db);
				break;
			case LOGIN:
				do_login(acceptfd, &msg, db);
				break;
			case ADD_IFO: 
				do_add_info(acceptfd, &msg, db);
				break;
			case MODIFY_IFO:
				do_modify_info(acceptfd, &msg, db);
				break;
			case DELETE_IFO:
				do_delete_info(acceptfd, &msg, db);
				break;
			case QUERY_IFO:
				do_query_info(acceptfd, &msg, db);
				break;
			default:
				printf("Invalid data msg \n");
		}
		
	}
	printf("客户端退出\n");
	close(acceptfd);
	exit(0);

	return 0;
	
}

void do_register(int acceptfd, MSG *msg, sqlite3 *db)
{
	char sql[128] = {};
	char *errmsg;
	printf("注册...\n");
	sprintf(sql, "insert into usr values('%s', %s);", msg->name, msg->data);
	printf("%s\n", sql);
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("%s\n", errmsg);
		strcpy(msg->data, "该姓名已注册");
	}
	else
	{
		printf("客户端注册成功！\n");
		strcpy(msg->data, "注册成功");

	}
	if(send(acceptfd, msg, sizeof(MSG), 0) < 0)
	{
		perror("fail to send");
		return;
	}
	return;
}

int do_login(int acceptfd, MSG *msg, sqlite3 *db)
{
	char sql[128] = {};
	char *errmsg;
	char **ptr_result;
	int nrow;
	int ncolumn;

	sprintf(sql, "select * from usr where name = '%s' and pass = '%s';", msg->name, msg->data);
	printf("%s\n", sql);

	if( sqlite3_get_table(db, sql, &ptr_result, &nrow, &ncolumn, &errmsg) != SQLITE_OK  )
	{
		printf("%s\n", errmsg);
		return -1;
	}
	else
	{
		printf("get_table ok\n");
	}
	//查询成功，数据库中拥有此用户
	if(nrow == 1)
	{
		strcpy(msg->data, "登录成功");
		send(acceptfd, msg, sizeof(MSG), 0);
		return 1;
	
	}
    if(nrow == 0) //密码或者用户名错误
	{
		strcpy(msg->data, "用户名或密码错误,请重新输入!");
		send(acceptfd, msg, sizeof(MSG), 0);
	}

	return 0;
}
int do_add_info(int acceptfd, MSG *msg, sqlite3 *db)
{
	printf("新增员工信息...\n");

	return 1;
}
int do_modify_info(int acceptfd, MSG *msg, sqlite3 *db)
{
	printf("修改员工信息...\n");

	return 1;
}
int do_delete_info(int acceptfd, MSG *msg, sqlite3 *db)
{
	printf("删除员工信息...\n");

	return 1;
}
int do_query_info(int acceptfd, MSG *msg, sqlite3 *db)
{
	printf("查询员工信息...\n");

	return 0;
}






