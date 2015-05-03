#ifndef __SQLITE_H__
#define __SQLITE_H__

int sqlite_find_user(sqlite3 *db, const char *name, char *passwd);
int sqlite_insert_user(sqlite3 *db, const char *name, const char *passwd);
int sqlite_find_word(sqlite3 *db, const char * word, char *explain);

#endif // __SQLITE_H__
