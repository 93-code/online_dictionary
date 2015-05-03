#ifndef	__CLIENT_H__

int send_fix_len(int sockfd, char *buf, int len);
int recv_fix_len(int sockfd, char *buf, int len);

void display_menu(void);
void register_input(char *name, char *password);
void word_input(char *word, int maxlen);

int recv_packet(int sockfd, char *packet);

int do_register(int sockfd);
int do_login(int sockfd);
int do_word(int sockfd);
int do_exit(int sockfd);
int do_heart(int sockfd);
void sig_timer(int signo);


#endif //__CLIENT_H__