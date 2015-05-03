/*
 * 实现目标:
 * 交互协议
 *
 * 实现步骤:
 *
 * 1 制定协议                                                       
 *   客户端-->服务端                                                
 *   +------------+------------+-----------------------------+------+
 *   | 包体长(4B) | 功能号(2B) |             包体            | 功能 |
 *   +------------+------------+--------------+--------------+------+
 *   |   "0064"   |    "01"    | 用户名(32)   | 密码(32)     | 注册 |
 *   +------------+------------+--------------+--------------+------+
 *   |   "0064"   |    "02"    | 用户名(32)   | 密码(32)     | 登陆 |
 *   +------------+------------+--------------+--------------+------|
 *   |            |    "03"    |            单词             | 查询 |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "04"    |                             | 退出 |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "05"    |                             | 心跳 |
 *   +------------+------------+-----------------------------+------|
 *                                                                  |
 *   服务端-->客户端                                                |
 *   +------------+------------+-----------------------------+------|
 *   | 包体长(4B) | 功能号(2B) |             包体            | 功能 |
 *   +------------+------------+-----------------------------+------|
 *   |   "0002"   |    "01"    |             结果            | 注册 |
 *   +------------+------------+-----------------------------+------|
 *   |   "0002"   |    "02"    |             结果            | 登陆 |
 *   +------------+------------+-------+---------------------+------|
 *   |            |    "03"    |  结果 |       解释          | 查询 |
 *   +------------+------------+-------+---------------------+------|
 *   |   "0002"   |    "04"    |            结果             | 退出 |
 *   +------------+------------+-----------------------------+------|
 *   |   "0000"   |    "05"    |                             | 心跳 |
 *   +------------+------------+-----------------------------+------|
 *                                                                  
 *   (1) 结果(2B)                                                   
 *       "00"-成功 "01"-已经注册 "02"-数据库异常 "03"-用户异常 "04"-单词不存在
 * 
 * 2. 实现协议
 *    2.1 包头打包/解包
 *    2.2 客户端-->服务端协议的打包/解包
 *        注册/登陆/查询/退出
 *    2.3 服务端-->客户端协议的打包/解包
 *        注册/登陆/查询/退出
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
 * @brief						将整数转换成len个字节的字符串
 * @param[out]			buf													接收字符串的缓冲
 * @param[in]				val													整数值
 * @param[in]				len													字节数
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
 * @brief						将指定长度的字符串转换成整数
 * @param[int]			buf													字符串
 * @param[in]				len													字节数
 * @return					整数
 */
int atoi_len(const char *buf, int len)
{
	char tmp[32];
	
	memcpy(tmp, buf, len);
	tmp[len] = '\0';
	
	return atoi(tmp);
}

// 2.1 包头打包/解包
int packet_pack_head(char *packet, int content_len, int func)
{
	int len = 0;
	
	// 包体长
	itoa_len(packet + len, content_len, LEN_PACKET_LEN);
	len += LEN_PACKET_LEN;
	
	// 功能号
	itoa_len(packet + len, func, LEN_PACKET_FUNC);
	len += LEN_PACKET_FUNC;
	
	return len;
}

int packet_unpack_head(const char *packet, int *func)
{
	int len = 0;
	int content_len;
	
	// 包体长
	content_len = atoi_len(packet + len, LEN_PACKET_LEN);
	len += LEN_PACKET_LEN;
	
	// 功能号
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

// 2.2 客户端-->服务端协议的打包/解包

// 注册
int packet_pack_reg_req(char *packet, const char *name, const char *password)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_REG);
	
	// 用户名
	strcpy(packet + len, name);
	len += LEN_USER_NAME;
	
	// 密码
	strcpy(packet + len, password);
	len += LEN_PASSWORD;
	
	// 包体长度
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_reg_req(const char *packet, char *name, char *password)
{
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 用户名
	memcpy(name, packet + len, LEN_USER_NAME);
	len += LEN_USER_NAME;
	
	// 密码
	memcpy(password, packet + len, LEN_PASSWORD);
	len += LEN_PASSWORD;
}

// 登陆
int packet_pack_login_req(char *packet, const char *name, const char *password)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_LOGIN);
	
	// 用户名
	strcpy(packet + len, name);
	len += LEN_USER_NAME;
	
	// 密码
	strcpy(packet + len, password);
	len += LEN_PASSWORD;
	
	// 包体长度
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_login_req(const char *packet, char *name, char *password)
{
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 用户名
	memcpy(name, packet + len, LEN_USER_NAME);
	len += LEN_USER_NAME;
	
	// 密码
	memcpy(password, packet + len, LEN_PASSWORD);
	len += LEN_PASSWORD;
}

// 查单词
int packet_pack_word_req(char *packet, const char *word)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_WORD);
	
	// 单词
	strcpy(packet + len, word);
	len += strlen(word);
	
	// 包体长度
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

void packet_unpack_word_req(const char *packet, char *word)
{
	int word_len = atoi_len(packet, LEN_PACKET_LEN);
	
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 单词
	memcpy(word, packet + len, word_len);
	len += word_len;
	word[word_len] = '\0';
}

// 退出
int packet_pack_exit_req(char *packet)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_EXIT);
	
	return len;
}

void packet_unpack_exit_req(const char *packet)
{
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 包体为空，但本函数仍然预留用于未来扩展
}



// 2.3 服务端-->客户端协议的打包/解包
// 注册
int packet_pack_reg_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_REG);
	
	// 结果
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// 包体长
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_reg_resp(const char *packet)
{
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	return atoi_len(packet + len, LEN_RESULT);
}

// 登陆
int packet_pack_login_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_LOGIN);
	
	// 结果打包到返回包
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// 包体长
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_login_resp(const char *packet)
{
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	return atoi_len(packet + len, LEN_RESULT);
}

// 查单词
int packet_pack_word_resp(char *packet, int result, const char *explain)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_WORD);
	
	// 结果
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	if (RET_SUCCESS == result){
		// 解释
		strcpy(packet + len, explain);
		len += strlen(explain);
	}
	
	// 包体长
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_word_resp(const char *packet, char *explain)
{
	int result;
	int explain_len;
	
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 结果
	result = atoi_len(packet + len, LEN_RESULT);
	len += LEN_RESULT;
	
	if (RET_SUCCESS == result){
		// 解释
		explain_len = atoi_len(packet, LEN_PACKET_LEN);
		memcpy(explain, packet + len, explain_len);
		explain[explain_len] = '\0';
	}
	
	return result;
}

// 退出
int packet_pack_exit_resp(char *packet, int result)
{
	int len = 0;
	
	len = packet_pack_head(packet, len, FUNC_EXIT);
	
	// 结果
	itoa_len(packet + len, result, LEN_RESULT);
	len += LEN_RESULT;
	
	// 包体长
	itoa_len(packet, len - LEN_PACKET_HEAD, LEN_PACKET_LEN);
	
	return len;
}

int packet_unpack_exit_resp(const char *packet)
{
	int result;
	
	// 包头接收时已经解析
	int len = LEN_PACKET_HEAD;
	
	// 结果
	result = atoi_len(packet + len, LEN_RESULT);
	len += LEN_RESULT;
	
	return result;
}


