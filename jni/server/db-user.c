#include "db-user.h"

#include "db-repo.h"

#include "logger.h"
#include "osapi.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

static const char *sql_create = "CREATE TABLE IF NOT EXISTS dbuser( \
        uid INTEGER PRIMARY KEY ASC,   \
        user TEXT,                    \
        user_from TEXT,                    \
        cred TEXT,                    \
        type INTEGER);";

static const char *sql_select =  "SELECT * FROM dbuser;";

static const char *sql_lookup = "SELECT * FROM dbuser WHERE user = ?1;";

static const char *sql_insert = "INSERT INTO dbuser VALUES(?1, ?2, ?3, ?4);";

static const char *sql_delete = "DELETE FROM dbuser WHERE user = ?1;";

static const char *sql_update = "REPLACE INTO dbuser VALUES(?1, ?2, ?3, ?4);";

int dbuser_init(dbuser_t *dbu, const char *dbuser){
    sqlite3_stmt *stmt;
    int rc;

    assert((dbu != NULL) && (dbuser != NULL));

    rc = sqlite3_open_v2(dbuser, &dbu->dbuser, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
    if(rc != 0){
        log_warn("open dbuser fail:%s\n", rc);
        
        return -EINVAL;
    }

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_create, strlen(sql_create), &stmt, NULL);
    if(rc != SQLITE_OK){
        log_warn("prepare dbuser create stmt fail:%d\n", rc);
        sqlite3_close(dbu->dbuser);
        
        return -EINVAL;
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        log_warn("execute create dbuser fail:%d\n", rc);
        sqlite3_finalize(stmt);
        sqlite3_close(dbu->dbuser);
        
        return -EINVAL;
    }

    sqlite3_finalize(stmt);

    pthread_mutex_init(&dbu->mtx, NULL);

    return 0;
}

int dbuser_fini(dbuser_t *dbu){
    if(dbu != NULL){
        sqlite3_close(dbu->dbuser);
        dbu->dbuser = NULL;
    }

    pthread_mutex_destroy(&dbu->mtx);

    return 0;
}

static int64_t dbuser_find(dbuser_t *dbu, char *user){
    sqlite3_stmt *stmt;
    int64_t uid;
    int rc;

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_lookup, strlen(sql_lookup), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_text(stmt, 1, user, strlen(user), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        sqlite3_finalize(stmt);
        return -1;
    }

    uid = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);
    
    return uid;
}

int64_t dbuser_add(dbuser_t *dbu, char *user, unsigned char *cred, int type){
    sqlite3_stmt *stmt;
    int64_t uid;
    int   rc;

    uid = dbuser_find(dbu, user);
    if(uid >= 0){
        log_warn("user:%s already exist!\n", user);
        return -EEXIST;
    }

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_insert, strlen(sql_insert), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_text(stmt, 2, user, strlen(user), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, (char *)cred, strlen((char*)cred), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, type);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        log_warn("inser user:%s fail!\n", user);
        sqlite3_finalize(stmt);
        return -1;
    }

    uid  = sqlite3_last_insert_rowid(dbu->dbuser);

    sqlite3_finalize(stmt);

    return uid;
}

int64_t dbuser_del(dbuser_t *dbu, char *user){
    sqlite3_stmt    *stmt;
    int64_t uid;
    int rc = 0;

    uid = dbuser_find(dbu, user);
    if(uid < 0){
        log_warn("user:%s not exist!\n", user);
        return -ENOENT;
    }

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_delete, strlen(sql_delete), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_text(stmt, 1, user, strlen(user), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        uid = -1;
    }

    sqlite3_finalize(stmt);

    return uid;
}

int64_t dbuser_update(dbuser_t *dbu, char *user, unsigned char *cred, int type){
    sqlite3_stmt    *stmt;
    int64_t uid;
    int   rc;

    uid = dbuser_find(dbu, user);
    if(uid < 0){
        log_warn("user:%s not exist!\n", user);
        return -ENOENT;
    }

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_update, strlen(sql_update), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, uid);
    sqlite3_bind_text(stmt, 2, user, strlen(user), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, (char*)cred, strlen((char *)cred), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, type);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        uid = -1;
    }

    sqlite3_finalize(stmt);

    return uid;
}

int dbuser_lookup(dbuser_t *dbu, char *user, ruser_t **ru){
    sqlite3_stmt *stmt;
    ruser_t *r;
    char *temp;
    int len;
    int rc;

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_lookup, strlen(sql_lookup), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_text(stmt, 1, user, strlen(user), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        sqlite3_finalize(stmt);
        return -1;
    }

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        sqlite3_finalize(stmt);
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    r->uid = sqlite3_column_int64(stmt, 0);

    temp = (char *)sqlite3_column_text(stmt, 1);
    len = sqlite3_column_bytes(stmt, 1);
    if(len >= (DBUSER_NAME_LEN - 1)){
        osapi_free(r);
        sqlite3_finalize(stmt);
        return -EINVAL;
    }
    strncpy(r->user, temp, len);

    temp = (char *)sqlite3_column_text(stmt, 2);
    len = sqlite3_column_bytes(stmt, 2);
    if(len >= (DBUSER_CRED_LEN - 1)){
        osapi_free(r);
        sqlite3_finalize(stmt);
        return -EINVAL;
    }
    strncpy((char*)r->cred, temp, len);

    r->type = sqlite3_column_int(stmt, 3);
    fixmap_node_init(&r->node);

    sqlite3_finalize(stmt);

    *ru = r;

    return 0;
}

int dbuser_list(dbuser_t *dbu, fixarray_t **users){
    sqlite3_stmt *stmt;
    fixarray_t *fa;
    ruser_t *ru;
    char *temp;
    int rc;
    int i;
    int len;

    rc = fixarray_create(DBUSER_MAX, &fa);
    if(rc != 0){
        return rc;
    }

    rc = sqlite3_prepare_v2(dbu->dbuser, sql_select, strlen(sql_select), &stmt, NULL);
    if(rc != SQLITE_OK){
        fixarray_destroy(fa);
        return -EINVAL;
    }

    for(i = 0; i < DBUSER_MAX; i++){
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_DONE){
            rc = 0;
            break;
        }

        if(rc != SQLITE_ROW){
            rc = -1;
            break;
        }

        ru = osapi_malloc(sizeof(*ru));
        if(ru == NULL){
            rc = -1;
            break;
        }
        memset(ru, 0, sizeof(*ru));

        ru->uid = sqlite3_column_int64(stmt, 0);

        temp = (char *)sqlite3_column_text(stmt, 1);
        len = sqlite3_column_bytes(stmt, 1);
        if(len >= (DBUSER_NAME_LEN - 1)){
            osapi_free(ru);
            rc = -1;
            break;
        }
        strncpy(ru->user, temp, len);

        temp = (char *)sqlite3_column_text(stmt, 2);
        len = sqlite3_column_bytes(stmt, 2);
        if(len >= (DBUSER_CRED_LEN - 1)){
            osapi_free(ru);
            rc = -1;
        }
        strncpy((char *)ru->cred, temp, len);

        ru->type = sqlite3_column_int(stmt, 3);
        fixmap_node_init(&ru->node);

        fixarray_set(fa, i, (void *)ru);
    }

    sqlite3_finalize(stmt);

    *users = fa;

    return rc;
}


