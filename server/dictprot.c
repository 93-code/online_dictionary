#include "dictprot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sqlite3.h"

//set/get包体长度
void packet_set_len(char *packet, int len)
{
    char buf[LEN_PACKET+1];
    sprintf(buf, "%4d", len);
    memcpy(packet, buf, LEN_PACKET);
}

int packet_get_len(const char *packet)
{
    char buf[LEN_PACKET+1];
    memcpy(buf, packet, LEN_PACKET);
    buf[LEN_PACKET] = '\0';

    return atoi(buf);
}

//set/get功能号
void packet_set_func(char *packet, int func)
{
    char buf[LEN_FUNC + 1];
    sprintf(buf, "%02d", func);
    memcpy(packet, buf, LEN_FUNC);
}
int packet_get_func(const char *packet)
{
    int func;
    char buf[LEN_FUNC+1];
    memcpy(buf, packet + LEN_PACKET, LEN_FUNC);
    buf[LEN_FUNC] = '\0';

    return atoi(buf);
}

//接受制定长度数据，并返回数据长度
int recv_fix_len(int sockfd, char *buf, int len)
{
    int ret = 0;
    int recv_len = 0;
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
int send_fix_len(int sockfd, const char *buf, int len)
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

//接受头部，从这中获得功能号 
int packet_recv_head(int sockfd, int *func)
{
    int ret = 0;
    char buf[LEN_HEAD];

    ret = recv_fix_len(sockfd, buf, LEN_HEAD);
    if (-1 == ret){
        goto exit;
    }
    *func = packet_get_func(buf);
    if (*func < FUNC_REG || *func > FUNC_END){
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}
//协议解析框架
int packet_recv_proc(int sockfd, sqlite3 *db)
{
    int ret = 0;
    int func;
    char packet[1024];

    //取得功能号
    ret = packet_recv_head(sockfd, &func);
    if (-1 == ret){
        goto exit;
    }
#ifdef __DEBUG__
    printf("rece: %d :: %d\n", ret, func);
#endif

    switch (func){
    case FUNC_REG:
        ret = server_exec_reg(sockfd, db);
        if (-1 == ret){
            perror("Fail to server_exec_reg");
            goto exit;
        }
        break;
    case FUNC_LOGIN:
        break;
    
    case FUNC_SEARCH:
        break;
    case FUNC_QUIT:
        break;
    case FUNC_BEAT:
        break;
    case FUNC_END:
        break;
    }
exit:
    return ret;
}

int client_exec_reg(int sockfd, const char *name, const char *passwd)
{
    char buf[LEN_USER_MSG + 1];
    char packet[1024];
    //打包用户名与密码
    packet_set_len(packet, LEN_USER_MSG);
    packet_set_func(packet, FUNC_REG);
    sprintf(buf, "%s", name);
    sprintf(buf+LEN_USER_NAME, "%s", passwd);
    buf[LEN_USER_MSG] = '\0';
    memcpy(packet, buf, LEN_USER_MSG);
    //客户端发送包头
    send_fix_len(sockfd, packet, LEN_HEAD);

    return 0;
}
int server_exec_reg(int sockfd, sqlite3 *db)
{
    char name[LEN_USER_NAME+1];
    char passwd[LEN_USER_PASS+1];
    char packet[LEN_USER_MSG + 1];
    
    recv_fix_len(sockfd, packet, LEN_USER_MSG);
    printf("packet:%s\n", packet);
    packet[LEN_USER_MSG] = '\0';
    printf("packet:%s\n", packet);

    memcpy(name, packet, LEN_USER_NAME);
    name[LEN_USER_NAME] = '\0';

    memcpy(passwd, packet + LEN_USER_NAME, LEN_USER_PASS);
    passwd[LEN_USER_PASS] = '\0';

    insert_user_db(db, name, passwd);

    return 0;
}


