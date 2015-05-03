#ifndef __SERVER_H__
#define	__SERVER_H__

int recv_fix_len(int sockfd, char *buf, int len);
int send_fix_len(int sockfd, char *buf, int len);

int recv_proc(int sockfd, sqlite3 *db);
int do_register(char *packet, sqlite3 *db);
int do_login(char *packet, sqlite3 *db);
int do_exit(char *packet);
int do_heart(char *packet);

#endif // __SERVER_H__
