/*
 * ʵ��Ŀ��:
 * ���ߵ��Ӵʵ䣬����:
 * 1. ע��
 * 2. ��¼
 * 3. ��ѯ
 * 4. �˳�
 * 5. �����쳣(��չ)
 *
 * ʵ�ֲ���:
 * 1. ����
 *    1.1 �����׽���
 *    1.2 ���ӷ����� 
 *    1.3 ָ�����ȵ������շ�
 *
 * 2. �û�����
 *    2.1 ��ʾ�˵�
 *    2.2 ע������
 *    2.3 ��½����
 *    2.4 �鵥������
 *
 * 3. Э��
 *    prot.c prot.h
 *
 * 4. ҵ����
 *    4.1 �������ݰ�
 *    4.2 ע��
 *    4.3 ��½
 *    4.4 �鵥��
 *    4.5 �˳�
 *    4.6 ����
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

// ��ʱ����Ҫʹ��
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
	
	// �󶨶�ʱ�ź�:�յ�SIGALRM�ź���ִ��sig_timer
	if (signal(SIGALRM, sig_timer) == SIG_ERR){
		perror("Fail to signal!");
		exit(EXIT_FAILURE);
	}

	// 1.1 �����׽���
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd){
		perror("Fail to socket.");
		exit(EXIT_FAILURE);
	}

	// 1.2 ���ӷ�����
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
		// ������ʱ
		alarm(2);
		
		if (!login){
			display_menu();
			scanf("%d", &select);
			
			// ����scanf�ڻ��������µĻس�
			while(getchar() != '\n');
		
			switch (select){
				case 1:        
						// ע��
						ret = do_register(sockfd);
					break;
					
				case 2:
					// ��½
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

	// �ر�socket
	close(sockfd);

	return 0;
}

// 1.3 ָ�����ȵ����ݷ���
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
	
	// ����������ʱ
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

// 2.1 ��ʾ�˵�
void display_menu(void)
{
	printf("1. register\n");
	printf("2. login\n");
	printf("3. exit\n");
	printf(">");
}

// 2.2 ע������
void register_input(char *name, char *password)
{
	do {
		printf("user: ");
        //-1 : ����'\0'��λ��; fgets : xxxx\n\0; 
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

// 2.3 ��½����
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

// 2.4 �鵥������
void word_input(char *word, int maxlen)
{
	do {
		printf("1. exit\n");
		printf("word>");
		
		fgets(word, maxlen - 2, stdin);
		word[strlen(word) - 1] = '\0';
		
	} while (strlen(word) == 0);
	
}


// 4.1 �������ݰ�
int recv_packet(int sockfd, char *packet)
{
	int ret = 0;
	int func;
	int len;
	
	do {
		// ���հ�ͷ
		ret = recv_fix_len(sockfd, packet, LEN_PACKET_HEAD);
		if (-1 == ret || 0 == ret){
			goto exit;
		}
		
		// ������ͷ:ȡ�ù��ܺź����ݳ���
		len = packet_unpack_head(packet, &func);
		if (FUNC_INVALID == func){
			// ���յ������, ��ս��ջ���
			while (recv(sockfd, packet, 1, 0) == 1);
			
			ret = 0;
			goto exit;
		}
		
		// ���������������½��գ��������������ڼ������״̬
	} while (FUNC_HEART == func); 
	
	// ���հ���
	ret = recv_fix_len(sockfd, packet + LEN_PACKET_HEAD, len);
	if (-1 == ret || 0 == ret){
		goto exit;
	}
	ret = len + LEN_PACKET_HEAD;
	
exit:
	return ret;
}

// 4.2 ע��
int do_register(int sockfd)
{
	int ret = 0;
	char name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	char packet[1024];
	
	register_input(name, password);
	
	// ��������:���
	ret = packet_pack_reg_req(packet, name, password);
	
#ifdef	__DEBUG__
	packet[ret] = '\0';
	printf("----------------------------------------\n");
	printf("%s: send : packet = %s\n", __func__, packet);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	// ��������
	ret = send_fix_len(sockfd, packet, ret);
	if (-1 == ret){
		goto exit;
	}
	
	// ���շ��ذ�
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

    //������ذ��������Ƿ�ע��ɹ�
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

// 4.3 ��½
int do_login(int sockfd)
{
	int ret = 0;
	char name[LEN_USER_NAME];
	char password[LEN_PASSWORD];
	char packet[1024];
	
    //��¼����
	login_input(name, password);
	
	// ��������
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
	
	// ���շ��ذ�
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

	// ���Ӧ��
	ret = packet_unpack_login_resp(packet);
	if (RET_SUCCESS == ret){
		ret = 1;
	} else {
		// ��½���󣬻ص����˵�
		packet_disp_err(ret);
		ret = 2;
	}
	
exit:
	return ret;
}

// 4.4 �鵥��
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
	
	// ������
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
	
	// ���շ��ذ�
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

	// ���Ӧ��
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

// 4.5 �˳�
int do_exit(int sockfd)
{
	int ret = 0;
	char packet[1024];
	
	// ������
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
	
	// ���շ��ذ�
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

	// ���Ӧ��
	ret = packet_unpack_exit_resp(packet);
	if (RET_SUCCESS == ret){
	} else {
		packet_disp_err(ret);
	}
	ret = 1;
	
exit:
	return ret;
}

// 4.6 ����
int do_heart(int sockfd)
{
	int ret = 0;
	char packet[1024];
	
	// ������
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
	// ��ʱ���ر��׽���
	do_heart(sockfd);
	alarm(2);
}
