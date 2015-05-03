/*
 * ʵ��Ŀ��:
 * ����Э��
 *
 * ʵ�ֲ���:
 *
 * 1 �ƶ�Э��                                                       
 *   �ͻ���-->�����                                                
 *   +------------+------------+-----------------------------+------+
 *   | ���峤(4B) | ���ܺ�(2B) |             ����            | ���� |
 *   +------------+------------+--------------+--------------+------+
 *   |   "0064"   |    "01"    | �û���(32)   | ����(32)     | ע�� |
 *   +------------+------------+--------------+--------------+------+
 *   |   "0064"   |    "02"    | �û���(32)   | ����(32)     | ��½ |
 *   +------------+------------+--------------+--------------+------|
 *   |            |    "03"    |            ����             | ��ѯ |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "04"    |                             | �˳� |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "05"    |                             | ���� |
 *   +------------+------------+-----------------------------+------|
 *                                                                  |
 *   �����-->�ͻ���                                                |
 *   +------------+------------+-----------------------------+------|
 *   | ���峤(4B) | ���ܺ�(2B) |             ����            | ���� |
 *   +------------+------------+-----------------------------+------|
 *   |   "0002"   |    "01"    |             ���            | ע�� |
 *   +------------+------------+-----------------------------+------|
 *   |   "0002"   |    "02"    |             ���            | ��½ |
 *   +------------+------------+-------+---------------------+------|
 *   |            |    "03"    |  ��� |       ����          | ��ѯ |
 *   +------------+------------+-------+---------------------+------|
 *   |   "0002"   |    "04"    |            ���             | �˳� |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "05"    |                             | ���� |
 *   +------------+------------+-----------------------------+------|
 *                                                                  
 *   (1) ���(2B)                                                   
 *       "00"-�ɹ� "01"-�Ѿ�ע�� "02"-���ݿ��쳣 "03"-�û��쳣 "04"-���ʲ�����
 * 
 * 2. ʵ��Э��
 *    2.1 ��ͷ���/���
 *    2.2 �ͻ���-->�����Э��Ĵ��/���
 *        ע��/��½/��ѯ/�˳�
 *    2.3 �����-->�ͻ���Э��Ĵ��/���
 *        ע��/��½/��ѯ/�˳�
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sqlite3.h>

#include "prot.h"

/*
 * @brief						������ת����len���ֽڵ��ַ���
 * @param[out]			buf													�����ַ����Ļ���
 * @param[in]				val													����ֵ
 * @param[in]				len													�ֽ���
 * @example         100 --> val  4 -->len  buf = "0100"
 */
void itoa_len(char *buf, int val, int len)
{
	char fmt[32];
	char tmp[32];
	
	sprintf(fmt, "%%0%dd", len);
	
#ifdef	__DEBUG__
	printf("----------------------------------------\n");
	printf("%s: fmt = %s\n", __func__, fmt);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	
	sprintf(tmp, fmt, val);
	memcpy(buf, tmp, len);
	
}

/*
 * @brief						��ָ�����ȵ��ַ���ת��������
 * @param[int]			buf													�ַ���
 * @param[in]				len													�ֽ���
 * @return					����
 */
int atoi_len(const char *buf, int len)
{
	char tmp[32];
	
	memcpy(tmp, buf, len);
	tmp[len] = '\0';
	
	return atoi(tmp);
}

// 2.1 ��ͷ���/���
int packet_pack_head(char *packet, int content_len, int func)
{
	int len = 0;
	
	// ���峤
	itoa_len(packet + len, content_len, LEN_PACKET_LEN);
	len += LEN_PACKET_LEN;
	
	// ���ܺ�
	itoa_len(packet + len, func, LEN_PACKET_FUNC);
	len += LEN_PACKET_FUNC;
	
	return len;
}

int packet_unpack_head(const char *packet, int *func)
{
	int len = 0;
	int content_len;
	
	// ���峤
	content_len = atoi_len(packet + len, LEN_PACKET_LEN);
	len += LEN_PACKET_LEN;
	
	// ���ܺ�
	*func = atoi_len(packet + len, LEN_PACKET_FUNC);
	len += LEN_PACKET_FUNC;
	
	if (*func <= FUNC_START || *func >= FUNC_END){
		*func = FUNC_INVALID;
	}
	
	return content_len;
}

void packet_disp_err(int err)
{	
	switch (err){
		case RET_ERR_USER_EXIST:
			printf("User exist!\n");
			break;
			
		case RET_ERR_DATABASE:
			printf("Database is error!\n");
			break;
			
		case RET_ERR_USER:
			printf("User name is not exist or password is error.\n");
			break;
			
		case RET_ERR_WORD:
			printf("Word is not exist.\n");
			break;
			
		default:
			break;
	}
}

// 2.2 �ͻ���-->�����Э��Ĵ��/���

// ע��
int packet_pack_reg_req(char *packet, const char *name, const char *password)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_REG);
	
	// �û���
	strcpy(packet + len, name);
	len += LEN_USER_NAME;
	
	// ����
	strcpy(packet + len, password);
	len += LEN_PASSWORD;
	
	// ���峤��
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_reg_req(const char *packet, char *name, char *password)
{
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// �û���
	memcpy(name, packet + len, LEN_USER_NAME);
	len += LEN_USER_NAME;
	
	// ����
	memcpy(password, packet + len, LEN_PASSWORD);
	len += LEN_PASSWORD;
}

// ��½
int packet_pack_login_req(char *packet, const char *name, const char *password)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_LOGIN);
	
	// �û���
	strcpy(packet + len, name);
	len += LEN_USER_NAME;
	
	// ����
	strcpy(packet + len, password);
	len += LEN_PASSWORD;
	
	// ���峤��
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_login_req(const char *packet, char *name, char *password)
{
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// �û���
	memcpy(name, packet + len, LEN_USER_NAME);
	len += LEN_USER_NAME;
	
	// ����
	memcpy(password, packet + len, LEN_PASSWORD);
	len += LEN_PASSWORD;
}

// �鵥��
int packet_pack_word_req(char *packet, const char *word)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_WORD);
	
	// ����
	strcpy(packet + len, word);
	len += strlen(word);
	
	// ���峤��
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_word_req(const char *packet, char *word)
{
	int word_len = atoi_len(packet, LEN_PACKET_LEN);
	
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// ����
	memcpy(word, packet + len, word_len);
	len += word_len;
	word[word_len] = '\0';
}

// �˳�
int packet_pack_exit_req(char *packet)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_EXIT);
	
	return len;
}

void packet_unpack_exit_req(const char *packet)
{
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// ����Ϊ�գ�����������ȻԤ������δ����չ
}



// 2.3 �����-->�ͻ���Э��Ĵ��/���
// ע��
int packet_pack_reg_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_REG);
	
	// ���
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// ���峤
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_reg_resp(const char *packet)
{
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	return atoi_len(packet + len, LEN_RESULT);
}

// ��½
int packet_pack_login_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_LOGIN);
	
	// �����������ذ�
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// ���峤
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_login_resp(const char *packet)
{
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	return atoi_len(packet + len, LEN_RESULT);
}

// �鵥��
int packet_pack_word_resp(char *packet, int result, const char *explain)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_WORD);
	
	// ���
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	if (RET_SUCCESS == result){
		// ����
		strcpy(packet + len, explain);
		len += strlen(explain);
	}
	
	// ���峤
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_word_resp(const char *packet, char *explain)
{
	int result;
	int explain_len;
	
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// ���
	result = atoi_len(packet + len, LEN_RESULT);
	len += LEN_RESULT;
	
	if (RET_SUCCESS == result){
		// ����
		explain_len = atoi_len(packet, LEN_PACKET_LEN);
		memcpy(explain, packet + len, explain_len);
		explain[explain_len] = '\0';
	}
	
	return result;
}

// �˳�
int packet_pack_exit_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_EXIT);
	
	// ���
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// ���峤
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_exit_resp(const char *packet)
{
	int result;
	
	// ��ͷ����ʱ�Ѿ�����
	int len = LEN_PACKET_HEAD;
	
	// ���
	result = atoi_len(packet + len, LEN_RESULT);
	len += LEN_RESULT;
	
	return result;
}


