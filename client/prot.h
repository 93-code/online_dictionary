#ifndef	__PROT_H__
#define __PROT_H__

#define	LEN_PACKET_LEN		4
#define LEN_PACKET_FUNC		2
#define	LEN_PACKET_HEAD		(LEN_PACKET_LEN + LEN_PACKET_FUNC)

#define LEN_USER_NAME			32
#define LEN_PASSWORD			32
#define LEN_RESULT				2

// ·µ»Ø½á¹û
#define RET_SUCCESS				    0
#define RET_ERR_USER_EXIST		1
#define	RET_ERR_DATABASE			2
#define	RET_ERR_USER					3
#define	RET_ERR_WORD					4

enum {
	FUNC_START = 0,
	FUNC_REG,
	FUNC_LOGIN,
	FUNC_WORD,
	FUNC_EXIT,
	FUNC_HEART,
	FUNC_END,
	FUNC_INVALID
};

int packet_pack_head(char *packet, int content_len, int func);
int packet_unpack_head(const char *packet, int *func);
void packet_disp_err(int err);

int packet_pack_reg_req(char *packet, const char *name, const char *password);
void packet_unpack_reg_req(const char *packet, char *name, char *password);

int packet_pack_login_req(char *packet, const char *name, const char *password);
void packet_unpack_login_req(const char *packet, char *name, char *password);

int packet_pack_word_req(char *packet, const char *word);
void packet_unpack_word_req(const char *packet, char *word);

int packet_pack_exit_req(char *packet);
void packet_unpack_exit_req(const char *packet);


int packet_pack_word_resp(char *packet, int result, const char *explain);
int packet_unpack_word_resp(const char *packet, char *explain);

int packet_pack_reg_resp(char *packet, int result);
int packet_unpack_reg_resp(const char *packet);

int packet_pack_login_resp(char *packet, int result);
int packet_unpack_login_resp(const char *packet);

int packet_pack_exit_resp(char *packet, int result);
int packet_unpack_exit_resp(const char *packet);

#endif // __PROT_H__
