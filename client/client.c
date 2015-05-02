/* 11:25 2015-04-30 Thursday */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dictprot.h"

int do_register(int sockfd)
{
    int ret = 0;
    char name[LEN_USER_NAME+1];
    char passwd[LEN_USER_PASS+1];
    fprintf(stdout, "Input your name and passwd\n");
    fprintf(stdout, "name: ");
    fgets(name, LEN_USER_NAME, stdin);
    name[LEN_USER_NAME] = '\0';
    fprintf(stdout, "passwd: ");
    fgets(passwd, LEN_USER_PASS, stdin);
    passwd[LEN_USER_PASS] = '\0';

    //注册
    ret = client_exec_reg(sockfd, name, passwd);
    if (-1 == ret){
        perror("Fail to send name and passwd");
    }
    return ret;
}

void do_quit(int sockfd)
{
    client_exec_quit(sockfd);
}
int main(int argc, const char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage : %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret;
    int len;
    int opt;
    int sockfd;
    char packet[1024];

    struct sockaddr_in server_addr;

    //创建客户端套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd){
        perror("Fail to socket");
        exit(EXIT_FAILURE);
    }

    //init server_addr
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret){
        perror("Fail to connect");
        exit(EXIT_FAILURE);
    }

#ifdef __DEBUG2__/*{{{*/
        client_exec_reg(sockfd, "hello", "3242342");
        len = recv(sockfd, packet, sizeof(packet), 0);
        packet[len] = '\0';
        printf("recv packet from server: %s\n", packet);
#endif/*}}}*/

    while (1){
        //输出提示界面
        puts("===========================");
        puts("== 1: register== 2: login==");
        puts("===========================");
        puts("== 3: search ==  4: quit===");
        puts("===========================");
        //opt
        scanf("%d", &opt);
        getchar();
        switch (opt){
        case 1:
            do_register(sockfd);
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            //未发信号给服务器，服务器进程未推迟
            do_quit(sockfd);
            goto exit;
            break;
        default:
            fprintf(stderr, "Input opt\n");
        }
    
    }
exit:
    close(sockfd);
    return 0;
}
