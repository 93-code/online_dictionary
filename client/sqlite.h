/* 15:05 2015-05-01 Friday */
#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

int open_db(const char *pathname, sqlite3 **db);
int search_user_db(sqlite3 *db, const char *name);
int insert_user_db(sqlite3 *db, const char *name, const char *passwd);
#endif
