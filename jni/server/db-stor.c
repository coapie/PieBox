

#include "db-stor.h"

#include "logger.h"
#include "osapi.h"

#include <assert.h>
#include <string.h>
#include <errno.h>

static const char *sql_fcreate = "CREATE TABLE IF NOT EXISTS dbstor( \
        fid INTEGER PRIMARY KEY ASC,    \
        folder INTEGER,                 \
        fowner INTEGER,                 \
        ftype INTEGER,                  \
        fname TEXT,                     \
        fpath TEXT,                     \
        fwhere TEXT,                    \
        fsnap BLOB);";

static const char *sql_finsert = "INSERT INTO dbstor VALUES(?1, ?2, ?3, ?4, \
                                        ?5, ?6, ?7, ?8);";
static const char *sql_fdelete = "DELETE FROM dbstor WHERE fid = ?1;";

static const char *sql_flookup = "SELECT * FROM dbstor WHERE fid = ?1;";

static const char *sql_flookup_byname  = "SELECT * FROM dbstor WHERE fname = ?1 \
                                AND folder = ?2 AND ftype = ?3;";

static const char *sql_fupdate = "REPLACE INTO dbstor VALUES(?1, ?2, ?3, \
                                    ?4, ?5, ?6, ?7, ?8);";

static const char *sql_lookup =  "SELECT * FROM dbstor \
                        WHERE folder = ?1 AND ftype = ?2  ORDER BY ?3;";

static int dbstor_init_root_folder(dbstor_t *dbs){
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_finsert, strlen(sql_finsert), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, 0);
    sqlite3_bind_int64(stmt, 2, -1); // root folder's container is not exist!
    sqlite3_bind_int64(stmt, 3, 0);
    sqlite3_bind_int(stmt, 4, DBSTOR_FOLDER_TYPE_BIT);
    
    sqlite3_bind_null(stmt, 5);
    sqlite3_bind_null(stmt, 6);
    sqlite3_bind_null(stmt, 7);
    sqlite3_bind_null(stmt, 8);
    
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int dbstor_init(dbstor_t *dbs, const char *dbname){
    sqlite3_stmt *stmt;
    int rc;

    assert((dbname != NULL) && (dbs != NULL));

    if(sqlite3_open_v2(dbname, &dbs->dbstor, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL) != 0){
        log_warn("open dbstor:%s fail\n", dbname);
        return -EINVAL;
    }

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_fcreate, strlen(sql_fcreate), &stmt, NULL);
    if(rc != SQLITE_OK){
        sqlite3_close(dbs->dbstor);
        return -EINVAL;
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        sqlite3_finalize(stmt);
        sqlite3_close(dbs->dbstor);
        return -EINVAL;
    }

    sqlite3_finalize(stmt);
    
    dbstor_init_root_folder(dbs);

    pthread_mutex_init(&dbs->mtx, NULL);

    return 0;
}

int dbstor_fini(dbstor_t *dbs){
    if(dbs->dbstor != NULL){
        sqlite3_close(dbs->dbstor);
        dbs->dbstor = NULL;
    }

    pthread_mutex_destroy(&dbs->mtx);

    return 0;
}

int dbstor_file_new(dbstor_file_t **file){
    dbstor_file_t *f;

    f = osapi_malloc(sizeof(*f));
    if(f == NULL){
        return -ENOMEM;
    }
    memset(f, 0, sizeof(*f));

    *file = f;
    return 0;
}

void dbstor_file_free(dbstor_file_t *file){
    if(file->fname != NULL){
        osapi_free(file->fname);
        file->fname = NULL;
    }

    if(file->fpath != NULL){
        osapi_free(file->fpath);
        file->fpath = NULL;
    }

    if(file->fwhere != NULL){
        osapi_free(file->fwhere);
        file->fwhere = NULL;
    }

    if(file->fsnap_len != 0){
        osapi_free(file->fsnap);
        file->fsnap = NULL;
        file->fsnap_len = 0;
    }

    osapi_free(file);
}

int dbstor_finsert(dbstor_t *dbs, dbstor_file_t *file){
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_finsert, strlen(sql_finsert), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_int64(stmt, 2, file->folder);
    sqlite3_bind_int64(stmt, 3, file->fowner);
    sqlite3_bind_int(stmt, 4, file->ftype);   
    
    sqlite3_bind_text(stmt, 5, file->fname, strlen(file->fname), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, file->fpath, strlen(file->fpath), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, file->fwhere, strlen(file->fwhere), SQLITE_STATIC);

    if(file->fsnap_len == 0){
        sqlite3_bind_null(stmt, 8);
    }else{
        sqlite3_bind_blob(stmt, 8, file->fsnap, file->fsnap_len, SQLITE_STATIC);
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        sqlite3_finalize(stmt);
        return -1;
    }

    file->fid = sqlite3_last_insert_rowid(dbs->dbstor);

    sqlite3_finalize(stmt);

    return 0;
}

int dbstor_fdelete(dbstor_t * dbs, int64_t fid){
    sqlite3_stmt *stmt;
    int rc = 0;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_fdelete, strlen(sql_fdelete), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, fid);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        rc = -1;
    }else{
        rc = 0;
    }

    sqlite3_finalize(stmt);

    return rc;
}

int dbstor_fupdate(dbstor_t * dbs, int64_t fid, dbstor_file_t *file){
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_fupdate, strlen(sql_fupdate), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, fid);
    sqlite3_bind_int64(stmt, 2, file->folder);
    sqlite3_bind_int64(stmt, 3, file->fowner);
    sqlite3_bind_int(stmt, 4, file->ftype);

    sqlite3_bind_text(stmt, 5, file->fname, strlen(file->fname), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, file->fpath, strlen(file->fpath), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, file->fwhere, strlen(file->fwhere), SQLITE_STATIC);

    if(file->fsnap_len == 0){
        sqlite3_bind_null(stmt, 8);
    }else{
        sqlite3_bind_blob(stmt, 8, file->fsnap, file->fsnap_len, SQLITE_STATIC);
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        rc = -1;
    }else{
        rc = 0;
    }

    sqlite3_finalize(stmt);

    return rc;
}

static int dbstor_file_from_stmt(sqlite3_stmt *stmt, dbstor_file_t *file){
    char *temp;
    int len;

    assert((stmt != NULL) && (file != NULL));

    memset(file, 0, sizeof(*file));

    file->fid = sqlite3_column_int64(stmt, 0);

    file->folder  = sqlite3_column_int64(stmt, 1);
    file->fowner  = sqlite3_column_int64(stmt, 2);
    file->ftype = sqlite3_column_int(stmt, 3);

    temp = (char *)sqlite3_column_text(stmt, 4);
    len = sqlite3_column_bytes(stmt, 4);
    if(len != 0){
        file->fname = osapi_malloc(len + 1);
        if(file->fname == NULL)
            return -ENOMEM;

        strncpy(file->fname, temp, len);
        file->fname[len] = 0;
    }else{
        file->fname =  NULL;
    }

    temp = (char *)sqlite3_column_text(stmt, 5);
    len = sqlite3_column_bytes(stmt, 5);
    if(len != 0){
        file->fpath = osapi_malloc(len + 1);
        if(file->fpath == NULL){
            osapi_free(file->fname);
            file->fname = NULL;
            return -ENOMEM;
        }

        strncpy(file->fpath, temp, len);
        file->fpath[len] = 0;
    }else{
        file->fpath =  NULL;
    }

    temp = (char *)sqlite3_column_text(stmt, 6);
    len = sqlite3_column_bytes(stmt, 6);
    if(len != 0){
        file->fwhere = osapi_malloc(len + 1);
        if(file->fwhere == NULL){
            osapi_free(file->fname);
            osapi_free(file->fpath);
            file->fname = NULL;
            file->fpath = NULL;
            return -ENOMEM;
        }

        strncpy(file->fwhere, temp, len);
        file->fwhere[len] = 0;
    }else{
        file->fwhere =  NULL;
    }

    file->fsnap_len = sqlite3_column_bytes(stmt, 7);
    if(file->fsnap_len > 0){
        file->fsnap = osapi_malloc(file->fsnap_len);
        if(file->fsnap == NULL){
            osapi_free(file->fname);
            osapi_free(file->fpath);
            osapi_free(file->fwhere);
            file->fname = NULL;
            file->fpath = NULL;
            file->fwhere = NULL;
            return -ENOMEM;
        }
        memcpy(file->fsnap, sqlite3_column_blob(stmt, 7), file->fsnap_len);
    }else{
        file->fsnap_len = 0;
        file->fsnap = NULL;
    }

    return 0;
}

int dbstor_flookup(dbstor_t * dbs, int64_t fid, dbstor_file_t **file){
    sqlite3_stmt *stmt;
    dbstor_file_t *f;
    int rc;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_flookup, strlen(sql_flookup), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, fid);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        sqlite3_finalize(stmt);
        return -1;
    }

    if(dbstor_file_new(&f) != 0){
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = dbstor_file_from_stmt(stmt, f);
    if(rc != 0){
        dbstor_file_free(f);
        rc = -EINVAL;
    }else{
        *file = f;
        rc = 0;
    }

    sqlite3_finalize(stmt);

    return rc;
}

int dbstor_flookup_byname(dbstor_t *dbs, char *fname, int64_t folder,
        int type, dbstor_file_t **file){
    sqlite3_stmt *stmt;
    dbstor_file_t *f;
    int rc;

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_flookup_byname,
            strlen(sql_flookup_byname), &stmt, NULL);
    if(rc != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_text(stmt, 1, fname, strlen(fname), SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, folder);
    sqlite3_bind_int(stmt, 3, type);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        sqlite3_finalize(stmt);
        return -1;
    }

    if(dbstor_file_new(&f) != 0){
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = dbstor_file_from_stmt(stmt, f);
    if(rc != 0){
        dbstor_file_free(f);
        rc = -EINVAL;
    }else{
        *file = f;
        rc = 0;
    }

    sqlite3_finalize(stmt);

    return rc;
}

int dbstor_lucreate(dbstor_t * dbs, int64_t folder, int type, int asc, dbstor_lookup_t **lu){
    dbstor_lookup_t *l;
    int rc;

    assert((dbs != NULL) && (lu != NULL));

    l = osapi_malloc(sizeof(*l));
    if(l == NULL){
        return -ENOMEM;
    }
    memset(l, 0, sizeof(*l));
    l->luid = (int64_t)l; // address as his id

    rc = sqlite3_prepare_v2(dbs->dbstor, sql_lookup, strlen(sql_lookup), &l->stmt, NULL);
    if(rc != SQLITE_OK){
        osapi_free(l);
        return -EINVAL;
    }

    sqlite3_bind_int64(l->stmt, 1, folder);
    sqlite3_bind_int(l->stmt, 2, type);
    sqlite3_bind_text(l->stmt, 3, asc ? "ASC " : "DESC", asc ? 3 : 4, SQLITE_STATIC);

    *lu = l;

    return 0;
}

int dbstor_ludestroy(dbstor_t *dbs, dbstor_lookup_t *lu){
    assert((lu != NULL) && (lu->stmt != NULL));

    sqlite3_finalize(lu->stmt);
    osapi_free(lu);

    return 0;
}

int dbstor_lunext(dbstor_t *dbs, dbstor_lookup_t *lu, int len, fixarray_t **fa){
    fixarray_t *f;
    dbstor_file_t *file;
    int i;
    int rc;

    assert((lu != NULL) && (fa != NULL));

    rc = fixarray_create(len, &f);
    if(rc != 0){
        return rc;
    }

    for(i = 0; i < len; i++){
        rc = sqlite3_step(lu->stmt);
        switch(rc){
        case SQLITE_DONE:
            /* no more record in db */
            rc = 0;
            break;

        case SQLITE_ROW:
            rc = dbstor_file_new(&file);
            if(rc != 0){
                break;
            }

            rc = dbstor_file_from_stmt(lu->stmt, file);
            if(rc != 0){
                dbstor_file_free(file);
                break;
            }

            fixarray_set(f, i, (void *)file);
            continue;

        default:
            rc = -1;
            break;
        }
        break;
    }

    *fa = f;

    return rc;
}


