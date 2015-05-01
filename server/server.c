/* 10:14 2015-04-30 Thursday */
/*
 *  实现目标：
 *  在线电子词典：
 *  1. 注册
 *  2. 登陆
 *  3. 查询
 *  4. 退出
 *  5. 网络异常
 *
 *  实现步骤：
 *  1. 网络
 *      1.1 准备网络，接受客户端连接
 *      1.2 为多个客户端服务（多进程）
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "dictprot.h"
#include "sqlite.h"

void handler_client(pid_t clientfd, sqlite3 *db)
{
    int len;
    int ret = 0;
    char packet[1024];
        //recv
        printf("In %s : %s\n", __FILE__, __FUNCTION__);
        ret = packet_recv_proc(clientfd, db);
        if (-1 == ret){
            perror("Fail to packet_recv_proc");
        }
        printf("In %s : %s\n", __FILE__, __FUNCTION__);
#ifdef __DEBUG2__
        len = recv(clientfd, packet, sizeof(packet), 0);
        printf("In %s:%s\n", __FILE__, __FUNCTION__);
        packet[len] = '\0';
        printf("packet : %s\n", packet);
        send(clientfd, packet, len, 0);
        printf("send over\n");
#endif
}

int main(int argc, const char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage : %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int ret;
    int sockfd;
    pid_t pid;
    int clientfd;
    sqlite3 *db;

    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr);
    //设置信号处理方式：显示忽略，自动回收资源
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR){
        perror("Fail to signal");
        exit(EXIT_FAILURE);
    }

    //创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd){
        perror("Fail to socket");
        exit(EXIT_FAILURE);
    }

    //bind 服务器
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret){
        perror("Fail to bind");
        exit(EXIT_FAILURE);
    }

    //监听套接字
    ret = listen(sockfd, 10);

    //打开数据库
    open_db("dict.db", &db);
    //accept 处理
    while (1){
        clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
        if (-1 == clientfd){
            perror("Fail to accept");
            break;
        }

        printf("========================\n");
        printf("ip      :%s\n", inet_ntoa(peer_addr.sin_addr));
        printf("port    :%d\n", ntohs(peer_addr.sin_port));
        printf("========================\n");

        //进程处理客户端
        pid = fork();
        if (-1 == pid){
            perror("Fail to fork");
            break;
        }
        //子进程接受客户端指令
        if (pid == 0){
#ifdef __DEBUG__
            printf("This is a child process\n");
#endif
            handler_client(clientfd, db);
        }

    }
    return 0;
}
