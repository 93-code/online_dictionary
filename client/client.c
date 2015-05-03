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
 *    1.2 连接服务器 
 *    1.3 指定长度的数据收发
 *
 * 2. 用户输入
 *    2.1 显示菜单
 *    2.2 注册输入
 *    2.3 登陆输入
 *    2.4 查单词输入
 *
 * 3. 协议
 *    prot.c prot.h
 *
 * 4. 业务处理
 *    4.1 接收数据包
 *    4.2 注册
 *    4.3 登陆
 *    4.4 查单词
 *    4.5 退出
 *    4.6 心跳
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "prot.h"
#include "client.h"

// 定时器需要使用
int sockfd;

// ./client 192.168.1.106 8887
int main(int argc, const char *argv[])
{
	int ret = 0;
	int len = 0;
	
	int select;
	int login = 0;
	
	struct sockaddr_in server_addr;
	
	if (argc < 3){
		printf("Usage: %s <server ip> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// 绑定定时信号:收到SIGALRM信号则执行sig_timer
	if (signal(SIGALRM, sig_timer) == SIG_ERR){
		perror("Fail to signal!");
		exit(EXIT_FAILURE);
	}

	// 1.1 创建套接字
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd){
		perror("Fail to socket.");
		exit(EXIT_FAILURE);
	}

	// 1.2 连接服务器
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (-1 == ret){
		perror("Fail to connect.");
		exit(EXIT_FAILURE);
	}
		
	while (1){
		// 启动定时
		alarm(2);
		
		if (!login){
			display_menu();
			scanf("%d", &select);
			
			// 处理scanf在缓存中留下的回车
			while(getchar() != '\n');
		
			switch (select){
				case 1:        
						// 注册
						ret = do_register(sockfd);
					break;
					
				case 2:
					// 登陆
					ret = do_login(sockfd);
					if (1 == ret) {
						login = 1;
					} 
					break;
					
				default:
					break;
			}
		} else {
			ret = do_word(sockfd);
			if (2 == ret){
				login = 0;
			}
		}
		
		if (3 == select){
			ret = do_exit(sockfd);
			break;
		}
		
		if (-1 == ret || 0 == ret){
			printf("Net is error!\n");
			break;
		}
	}

	// 关闭socket
	close(sockfd);

	return 0;
}

// 1.3 指定长度的数据发收
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
		} else if (0 == ret){
			printf("recv timeout!\n");
			goto exit;
		}
		
		
		recv_len += ret;
	}
	ret = recv_len;
	
exit:
	return ret;
}

// 2.1 显示菜单
void display_menu(void)
{
	printf("1. register\n");
	printf("2. login\n");
	printf("3. exit\n");
	printf(">");
}

// 2.2 注册输入
void register_input(char *name, char *password)
{
	do {
		printf("user: ");
        //-1 : 保留'\0'的位置; fgets : xxxx\n\0; 
		fgets(name, LEN_USER_NAME - 1, stdin);
		name[strlen(name) - 1] = '\0';
		
		printf("password: ");
		fgets(password, LEN_PASSWORD - 1, stdin);
		password[strlen(password) - 1] = '\0';
	} while (strlen(name) == 0 || strlen(password) == 0);
	
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s: name = %s, password = %s\n", __func__, name, password);
	printf("----------------------------------------\n");
	printf("\n");
#endif // __DEBUG__
}

// 2.3 登陆输入
void login_input(char *name, char *password)
{
	do {
		printf("login: ");
		fgets(name, LEN_USER_NAME - 1, stdin);
		name[strlen(name) - 1] = '\0';
		
		printf("password: ");
		fgets(password, LEN_PASSWORD - 1, stdin);
		password[strlen(password) - 1] = '\0';
	} while (strlen(name) == 0 || strlen(password) == 0);
	
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s: name = %s, password = %s\n", __func__, name, password);
	printf("----------------------------------------\n");
	printf("\n");
#endif // __DEBUG__
}

// 2.4 查单词输入
void word_input(char *word, int maxlen)
{
	do {
		printf("1. exit\n");
		printf("word>");
		
		fgets(word, maxlen - 2, stdin);
		word[strlen(word) - 1] = '\0';
		
	} while (strlen(word) == 0);
	
}


// 4.1 接收数据包
int recv_packet(int sockfd, char *packet)
{
	int ret = 0;
	int func;
	int len;
	
	do {
		// 接收包头
		ret = recv_fix_len(sockfd, packet, LEN_PACKET_HEAD);
		if (-1 == ret || 0 == ret){
			goto exit;
		}
		
		// 解析包头:取得功能号和内容长度
		len = packet_unpack_head(packet, &func);
		if (FUNC_INVALID == func){
			// 接收到错误包, 清空接收缓冲
			while (recv(sockfd, packet, 1, 0) == 1);
			
			ret = 0;
			goto exit;
		}
		
		// 接收心跳包，重新接收，心跳包仅仅用于检测网络状态
	} while (FUNC_HEART == func); 
	
	// 接收包体
	ret = recv_fix_len(sockfd, packet + LEN_PACKET_HEAD, len);
	if (-1 == ret || 0 == ret){
		goto exit;
	}
	ret = len + LEN_PACKET_HEAD;
	
exit:
	return ret;
}

// 4.2 注册
int do_register(int sockfd)
{
	int ret = 0;
	char name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	char packet[1024];
	
	register_input(name, password);
	
	// 发送请求:打包
	ret = packet_pack_reg_req(packet, name, password);
	
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	// 发送请求
	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
	// 接收返回包
	ret = recv_packet(sockfd, packet);
	if (ret <= 0){
		goto exit;
	}
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: recv : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

    //解包返回包，分析是否注册成功
	ret = packet_unpack_reg_resp(packet);
	if (RET_SUCCESS == ret){
		printf("Register OK!\n");
	} else {
		packet_disp_err(ret);
	}
	
	ret = 1;
	
exit:
	return ret;
}

// 4.3 登陆
int do_login(int sockfd)
{
	int ret = 0;
	char name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	char packet[1024];
	
    //登录输入
	login_input(name, password);
	
	// 请求包打包
	ret = packet_pack_login_req(packet, name, password);
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
	// 接收返回包
	ret = recv_packet(sockfd, packet);
	if (ret <= 0){
		goto exit;
	}
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: recv : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 解回应包
	ret = packet_unpack_login_resp(packet);
	if (RET_SUCCESS == ret){
		ret = 1;
	} else {
		// 登陆错误，回到主菜单
		packet_disp_err(ret);
		ret = 2;
	}
	
exit:
	return ret;
}

// 4.4 查单词
int do_word(int sockfd)
{
	int ret = 0;
	char word[512];
	char packet[1024];
	char explain[1024];
	
	word_input(word, sizeof(word));
	
	if ('1' == word[0] && '\0' == word[1]){
		ret = 2;
		goto exit;
	}
	
	// 请求打包
	ret = packet_pack_word_req(packet, word);
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
	// 接收返回包
	ret = recv_packet(sockfd, packet);
	if (ret <= 0){
		goto exit;
	}
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: recv : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 解回应包
	ret = packet_unpack_word_resp(packet, explain);
	if (RET_SUCCESS == ret){
		printf("%s\n", explain);
	} else {
		packet_disp_err(ret);
	}
	ret = 1;
	
exit:
	return ret;
}

// 4.5 退出
int do_exit(int sockfd)
{
	int ret = 0;
	char packet[1024];
	
	// 请求打包
	ret = packet_pack_exit_req(packet);
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
	// 接收返回包
	ret = recv_packet(sockfd, packet);
	if (ret <= 0){
		goto exit;
	}
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: recv : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	// 解回应包
	ret = packet_unpack_exit_resp(packet);
	if (RET_SUCCESS == ret){
	} else {
		packet_disp_err(ret);
	}
	ret = 1;
	
exit:
	return ret;
}

// 4.6 心跳
int do_heart(int sockfd)
{
	int ret = 0;
	char packet[1024];
	
	// 请求打包
	ret = packet_pack_head(packet, 0, FUNC_HEART);
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
exit:
	return ret;
}

void sig_timer(int signo)
{
	// 超时，关闭套接字
	do_heart(sockfd);
	alarm(2);
}
