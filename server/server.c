/*
 * 实现目标:
 * 在线电子词典，具体:
 * 1. 注册
 * 2. 登录
 * 3. 查询
 * 4. 退出
 * 5. 网络异常(扩展)
 *
 * 实现步骤:
 * 1. 网络
 *    1.1 创建套接字
 *    1.2 绑定(本机)IP地址和(本软件)端口号
 *    1.3 设置为监听模式
 *    1.4 接受客户端连接
 *    1.5 支持为多客户端服务(多进程)
 *    1.6 指定长度的数据收发     
 *
 * 2. 数据库
 *    sqlite.c sqlite.h
 *
 * 3. 协议
 *    prot.c prot.h
 *
 * 4. 业务处理
 *    4.1 协议接收框架
 *    4.2 注册
 *    4.3 登陆
 *    4.4 查单词
 *    4.5 退出
 *    4.6 心跳
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <signal.h>
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <sqlite3.h>

#include "prot.h"
#include "sqlite.h"
#include "server.h"

void do_client(int sockfd, sqlite3 *db)
{
	int ret = 0;
	int len = 0;
	
	while (1){
		ret = recv_proc(sockfd, db);
		if (-1 == ret || 0 == ret){
			break;
		}
	}
	
	close(sockfd);
	exit(EXIT_SUCCESS);
}

// ./server 192.168.1.106 8887
int main(int argc, const char *argv[])
{
	int ret = 0;
	int len = 0;
	
	int sockfd;
	int clientfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in peer_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	
	pid_t pid;
	sqlite3 *db;
	
	if (argc < 3){
		printf("Usage: %s <ip> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// 告诉Linux内核, 子进程结束的时候, 系统自动回收资源
	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR){
		perror("Fail to signal");
		exit(EXIT_FAILURE);
	}
	
	// 1.1 创建套接字
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd){
		perror("Fail to socket.");
		exit(EXIT_FAILURE);
	}
	
	// 1.2 绑定(本机)IP地址和(本软件)端口号
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	ret = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (-1 == ret){
		perror("Fail to bind.");
		exit(EXIT_FAILURE);
	}
	
	// 1.3 设置为监听模式
	ret = listen(sockfd, 10);
	if (-1 == ret){
		perror("Fail to listen.");
		exit(EXIT_FAILURE);
	}
	
	ret = sqlite3_open("dict.db", &db);
	if (ret != SQLITE_OK){
		fprintf(stderr, "Fail to sqlite3_get_table : %s\n", sqlite3_errmsg(db));
		ret = EXIT_FAILURE;
		goto exit;
	}
		
	while (1) {
		// 1.4 接受客户端连接
		clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
		if (-1 == clientfd){
			perror("Fail to accept.");
			ret = EXIT_FAILURE;
			break;
		}
		
#ifdef __DEBUG__
		printf("-----------------------------------------\n");
		printf("ip   : %s\n", inet_ntoa(peer_addr.sin_addr));
		printf("port : %d\n", ntohs(peer_addr.sin_port));
		printf("-----------------------------------------\n");
#endif // __DEBUG__
		
		// 1.5 为多客户端服务(多进程)
		pid = fork();
		if (pid < 0){
			ret = EXIT_FAILURE;
			break;
		}
		
		if (0 == pid){
			// 子进程
			do_client(clientfd, db);
		}
		
	}
	
exit:
	sqlite3_close(db);
	close(sockfd);
	
	return ret;
}

// 1.6 指定长度的数据发收
int send_fix_len(int sockfd, char *buf, int len)
{
	int ret = 0;
	int send_len = 0;
	
	while (send_len < len){
		ret = send(sockfd, buf + send_len, len - send_len, 0);
		if (-1 == ret){
			perror("Fail to send");
			goto exit;
		}
		
		send_len += ret;
	}
	ret = send_len;
	
exit:
	return ret;
}

int recv_fix_len(int sockfd, char *buf, int len)
{
	int ret = 0;
	int recv_len = 0;
	struct timeval tv = {6, 0};
	
	// 用于心跳定时
	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if (ret < 0){
		perror("Fail to setsockopt.");
		goto exit;
	}
	
	while (recv_len < len){
		ret = recv(sockfd, buf + recv_len, len - recv_len, 0);
		if (-1 == ret){
			perror("Fail to recv");
			goto exit;
		}
		
		
		recv_len += ret;
	}
	ret = recv_len;
	
exit:
	return ret;
}

// 4.1 协议接收框架
int recv_proc(int sockfd, sqlite3 *db)
{
	int ret = 0;
	int func;
	char packet[1024];
	char packet_content_len;
	
	// 接收包头
	ret = recv_fix_len(sockfd, packet, LEN_PACKET_HEAD);
	if (-1 == ret || 0 == ret){
		goto exit;
	}
	
	// 解析包头
	packet_content_len = packet_unpack_head(packet, &func);
	if (FUNC_INVALID == func){
		ret = -1;
		goto exit;
	}
	
#ifdef	__DEBUG__
	printf("----------------------------------------\n");
	printf("%s: recv : len = %d, func = %d\n", __func__, ret, func);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	
	// 接收包体
	if (packet_content_len > 0){
		ret = recv_fix_len(sockfd, packet + LEN_PACKET_HEAD, packet_content_len);
		if (-1 == ret || 0 == ret){
			goto exit;
		}
	}
	
#ifdef	__DEBUG__
	packet[packet_content_len + LEN_PACKET_HEAD] = '\0';
	printf("----------------------------------------\n");
	printf("%s: recv : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	switch (func){
		case FUNC_REG:
			ret = do_register(packet, db);
			break;
			
		case FUNC_LOGIN:
			ret = do_login(packet, db);
			break;
			
		case FUNC_WORD:
			ret = do_word(packet, db);
			break;
			
		case FUNC_EXIT:
			ret = do_exit(packet);
			break;
			
		default:
			ret = do_heart(packet);
			break;
	}
	
	// 发送返回给客户端的数据
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	ret = send_fix_len(sockfd, packet, ret);
	
	// 退出功能
	if (FUNC_EXIT == func) ret = -1;

exit:
	return ret;
}

// 4.2 注册
int do_register(char *packet, sqlite3 *db)
{
	int ret = 0;
	char user_name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	
	// 解包
	packet_unpack_reg_req(packet, user_name, password);
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s : user_name = %s, password = %s\n", __func__, user_name, password);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 插入用户
	ret = sqlite_insert_user(db, user_name, password);
	if (ret > 0){
		// 用户已经存在
		ret = RET_ERR_USER_EXIST;
	} else if (0 == ret){
		// 添加用户成功
		ret = RET_SUCCESS;
	} else if (ret < 0){
		// 数据库异常
		ret = RET_ERR_DATABASE;
	}
	
	// 打回应包
	return packet_pack_reg_resp(packet, ret);
}

// 4.3 登陆
int do_login(char *packet, sqlite3 *db)
{
	int ret = 0;
	char user_name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	char tmp[LEN_PASSWORD];
	
	// 解包
	packet_unpack_login_req(packet, user_name, password);
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s : user_name = %s, password = %s\n", __func__, user_name, password);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 查找用户:并通过参数传回passwd
	ret = sqlite_find_user(db, user_name, tmp);
	if (ret > 0){
		// 用户存在
		if (strcmp(tmp, password) == 0){
			// 用户信息完全正确
			ret = RET_SUCCESS;
		} else {
			// 密码出错
			ret = RET_ERR_USER;
		}
		
	} else if (0 == ret){
		// 用户不存在
		ret = RET_ERR_USER;
	} else if (ret < 0){
		// 数据库异常
		ret = RET_ERR_DATABASE;
	}
	
	// 打回应包
	return packet_pack_login_resp(packet, ret);
}

// 4.4 查单词
int do_word(char *packet, sqlite3 *db)
{
	int ret = 0;
	char word[1024];
	char explain[1024];
	
	// 解包
	packet_unpack_word_req(packet, word);
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s : word = %s\n", __func__, word);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 插入用户
	ret = sqlite_find_word(db, word, explain);
	if (ret > 0){
		ret = RET_SUCCESS;
	} else if (0 == ret){
		// 单词不存在
		ret = RET_ERR_WORD;
	} else if (ret < 0){
		// 数据库异常
		ret = RET_ERR_DATABASE;
	}
	
	// 打回应包
	return packet_pack_word_resp(packet, ret, explain);
}

// 4.5 退出
int do_exit(char *packet)
{
	char explain[1024];
	
	// 解包
	packet_unpack_exit_req(packet);
	
	// 打回应包
	return packet_pack_exit_resp(packet, RET_SUCCESS);
}

// 4.6 心跳
int do_heart(char *packet)
{
	// 原包发送回去
	return LEN_PACKET_HEAD;
}
