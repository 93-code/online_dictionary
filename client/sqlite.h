/* 15:05 2015-05-01 Friday */
#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

int open_db(const char *pathname, sqlite3 **db);
int sum_db(sqlite3 *db);
int search_user_db(sqlite3 *db, const char *name, const char *passwd);
int insert_user_db(sqlite3 *db, const char *name, const char *passwd);
int search_word_db(sqlite3 *db, const char *word, char *explain);
#endif
