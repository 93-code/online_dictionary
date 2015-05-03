/*
 * 实现目标:
 * 数据库操作
 *
 * 实现步骤
 * 1. 获取用户信息
 * 2. 插入用户
 * 3. 找到单词，获得解释
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sqlite3.h>

#ifdef __DEBUG__
void sqlite_disp_debug(char **resultp, int row, int column)
{
	int i, j;
	
	printf("----------------------------------------\n");
	printf("%s: \n", __func__);
	
	for (i = 0; i <= row; i++){
		for (j = 0; j < column; j++){
			printf("%s ", resultp[i * column + j]);
		}
		printf("\n");
	}
	
	printf("----------------------------------------\n");
	printf("\n");
}
#endif // __DEBUG__

// 1. 获取用户信息
int sqlite_find_user(sqlite3 *db, const char *name, char *passwd)
{
	int ret = 0;
	char sql[512];
	char **resultp;
	char *errmsg;
	int row, column;
	
	sprintf(sql, "select password from users where name='%s';", name);
	
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s: sql = %s\n", __func__, sql);
	printf("----------------------------------------\n");
	printf("\n");
#endif
	
	ret = sqlite3_get_table(db, sql, &resultp, &row, &column, &errmsg);
	if (ret != SQLITE_OK){
		fprintf(stderr, "Fail to sqlite3_get_table : %s\n", errmsg);
		ret = -1;
		goto exit;
	}
	
#ifdef __DEBUG__
	sqlite_disp_debug(resultp, row, column);
#endif // __DEBUG__

	if (row >= 1){
		strcpy(passwd, resultp[1]);
	}
	ret = row;
	
	sqlite3_free_table(resultp);
	
exit:
	return ret;
}

// 2. 插入用户
int sqlite_insert_user(sqlite3 *db, const char *name, const char *passwd)
{
	int ret = 0;
	char password[128];
	char sql[512];
	char *errmsg;
	
	ret = sqlite_find_user(db, name, password);
	if (ret >= 1){
		goto exit;
	}
	
	// 插入
	sprintf(sql, "insert into users values('%s','%s');", name, passwd);
	
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s: sql = %s\n", __func__, sql);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK){
		fprintf(stderr, "Fail to sqlite3_exec : %s\n", errmsg);
		ret = -1;
		goto exit;
	}
	ret = 0;
	
exit:
	return ret;
}

// 3. 找到单词，获得解释
int sqlite_find_word(sqlite3 *db, const char * word, char *explain)
{
	int ret = 0;
	char password[128];
	char sql[512];
	char *errmsg;
	char **resultp;
	int row, column;
	
	sprintf(sql, "select explain from words where word='%s';", word);
	
#ifdef __DEBUG__
	printf("----------------------------------------\n");
	printf("%s: sql = %s\n", __func__, sql);
	printf("----------------------------------------\n");
	printf("\n");
#endif

	ret = sqlite3_get_table(db, sql, &resultp, &row, &column, &errmsg);
	if (ret != SQLITE_OK){
		fprintf(stderr, "Fail to sqlite3_get_table : %s\n", errmsg);
		ret = -1;
		goto exit;
	}
	
#ifdef __DEBUG__
	sqlite_disp_debug(resultp, row, column);
#endif // __DEBUG__

	// 获得解释
	if (row >= 1){
		strcpy(explain, resultp[1]);
	}
	ret = row;

	sqlite3_free_table(resultp);
exit:
	return ret;
}

#if 0
int main(void)
{
	int ret = 0;
	sqlite3 *db;
	char password[128];
	char explain[1024];
	
	ret = sqlite3_open("dict.db", &db);
	if (ret != SQLITE_OK){
		fprintf(stderr, "Fail to sqlite3_get_table : %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
	
	printf("%d\n", sqlite_find_user(db, "jacky", password));
	printf("%d\n", sqlite_insert_user(db, "tom", "123"));
	printf("%d\n", sqlite_find_word(db, "a", explain));
	
	
	sqlite3_close(db);
}
#endif
