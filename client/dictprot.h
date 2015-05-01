/* 13:09 2015-04-30 Thursday */
#ifndef __DICTPROT_H__
#define __DICTPROT_H__

#include "dictprot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sqlite.h"

#define LEN_FUNC        2
#define LEN_PACKET      4
#define LEN_HEAD        (LEN_FUNC + LEN_PACKET)
#define LEN_USER_NAME  32
#define LEN_USER_PASS  32
#define LEN_USER_MSG   (LEN_USER_NAME + LEN_USER_PASS)



enum{
    FUNC_REG = 1,
    FUNC_LOGIN,
    FUNC_SEARCH,
    FUNC_QUIT,
    FUNC_BEAT,
    FUNC_END
};

enum{
    RET_SUCCESS = 0,
    RET_REGISTER,
    RET_BADDATA,
    RET_BADUSER,
    RET_NOWORD,
    RET_END
};

//打包/解包
void packet_set_len(char *packet, int len);
int packet_get_len(const char *packet);
int packet_get_func(const char *packet);
int recv_fix_len(int sockfd, char *buf, int len);
int send_fix_len(int sockfd, const char *buf, int len);

int packet_recv_head(int sockfd, int *func);
int packet_recv_proc(int sockfd, sqlite3 *db);

int client_exec_reg(int sockfd, const char *name, const char *passwd);
int server_exec_reg(int sockfd, sqlite3 *db, int len);



#endif

