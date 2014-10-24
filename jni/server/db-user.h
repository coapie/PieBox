
#ifndef __DB_USER_H__
#define __DB_USER_H__

#include "fixarray.h"
#include "fixmap.h"

#include "sqlite3.h"

#include <stdint.h>
#include <pthread.h>

#define DBUSER_MAX               64
#define DBUSER_NAME_LEN          128
#define DBUSER_CRED_LEN          16
#define DBUSER_FROM_LEN          32
typedef struct _repo_user{
    fixmap_node_t node;

    int64_t uid;
    char user[DBUSER_NAME_LEN];
    unsigned char cred[DBUSER_CRED_LEN];
    int type;

    // don't store in db
    char from[DBUSER_FROM_LEN];
    unsigned char seed[DBUSER_CRED_LEN];
}ruser_t;

typedef struct _dbuser{
    pthread_mutex_t mtx;
    sqlite3 *dbuser;
}dbuser_t;

int dbuser_init(dbuser_t *dbu, const char *dbname);
int dbuser_fini(dbuser_t *dbu);

int dbuser_lookup(dbuser_t *dbu, char *user, ruser_t **ru);
int64_t dbuser_add(dbuser_t *dbu, char *user, unsigned char *cred, int type);
int64_t dbuser_del(dbuser_t *dbu, char *user);
int64_t dbuser_update(dbuser_t *dbu, char *user, unsigned char *cred, int type);
int dbuser_list(dbuser_t *dbu, fixarray_t **users);

#endif // __DB_USER_H__

