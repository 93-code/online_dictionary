/*
 *  2. 数据库
 *      2.1 打开，关闭
 *      2.2 查找用户信息
 *      2.3 插入用户信息(检查用户是否存在)
 *      2.4 找到单词，获得解释
 */
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

int open_db(const char *pathname, sqlite3 **db)
{
    int ret = 0;
    ret = sqlite3_open(pathname, db);
    if (ret != SQLITE_OK){
        fprintf(stderr, "Fail to open : %s\n", sqlite3_errmsg(*db));
        exit(EXIT_FAILURE);
    }
    return 0;
}

int search_user_db(sqlite3 *db, const char *name)
{
    int ret = 0;
    char **resultp; 
    int nrow;//记录数，有多少条符合要求的记录 
    int ncolumn;
    char *errmsg;
    char sql[1024];
    sprintf(sql, "select * from users where name='%s';", name);
#ifdef __DEBUG__
    puts(sql);
#endif 
    /*ret = sqlite3_exec(db, sql, NULL, NULL, NULL);*/
    ret = sqlite3_get_table(db, sql, &resultp, &nrow, &ncolumn, &errmsg);
    if (ret != SQLITE_OK){
        ret = -1;
        goto exit;
    }
    ret = nrow;
    sqlite3_free_table(resultp);
    
exit:
    return ret;
}

int insert_user_db(sqlite3 *db, const char *name, const char *passwd)
{
    int ret = 0;
    char sql[1024];

    char *errmsg;
    ret = search_user_db(db, name);
    if (ret >= 1){
        printf("用户已注册\n");
        goto exit;
    }
    //插入用户信息
    sprintf(sql, "insert into users values(%d,'%s','%s');", ++ret, name, passwd);
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK){
        fprintf(stderr, "Fail to sqlite3_exec : %s\n", errmsg);
        exit(EXIT_FAILURE);
    }
exit:
    return 0;
}

#ifdef __DEBUG2__
int main(int argc, const char *argv[])
{
    sqlite3 *db;

    open_db(argv[1], &db);
    insert_user_db(db, "yang", "12345");
    return 0;
}
#endif