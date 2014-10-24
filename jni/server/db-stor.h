
#ifndef __DBSTOR_H__
#define __DBSTOR_H__

#include "fixarray.h"
#include "fixmap.h"

#include "sqlite3.h"

#include <stdint.h>
#include <pthread.h>

typedef enum{
    DBSTOR_FILE_TYPE_VIDEO = 0,
    DBSTOR_FILE_TYPE_AUDIO,
    DBSTOR_FILE_TYPE_PHOTO,
    DBSTOR_FILE_TYPE_DOCS,
}dbstor_file_type_t;

#define DBSTOR_FOLDER_TYPE_BIT  0x00008000

typedef struct _dbstor_file{
    uint64_t fid;
    uint64_t folder;
    uint64_t fowner;

    int ftype;

    char *fname;
    char *fpath;
    char *fwhere;

    int fsnap_len;
    void *fsnap;
}dbstor_file_t;

typedef struct _dbstor_lookup{
    fixmap_node_t node;

    int64_t luid;
    sqlite3_stmt *stmt;
}dbstor_lookup_t;

typedef struct _dbstor{
    pthread_mutex_t mtx;
    sqlite3 *dbstor;
}dbstor_t;

int dbstor_init(dbstor_t *dbs, const char *dbname);
int dbstor_fini(dbstor_t *dbs);

int dbstor_file_new(dbstor_file_t **file);
void dbstor_file_free(dbstor_file_t *file);

int dbstor_finsert(dbstor_t *dbs, dbstor_file_t *file);
int dbstor_fdelete(dbstor_t *dbs, int64_t fid);
int dbstor_fupdate(dbstor_t *dbs, int64_t fid, dbstor_file_t *file);
int dbstor_flookup(dbstor_t *dbs, int64_t fid, dbstor_file_t **file);
int dbstor_flookup_byname(dbstor_t *dbs, char *fname, int64_t folder,
        int type, dbstor_file_t **file);

int dbstor_lucreate(dbstor_t *dbs, int64_t folder, int type, int asc, dbstor_lookup_t **lu);
int dbstor_ludestroy(dbstor_t *dbs, dbstor_lookup_t *lu);
int dbstor_lunext(dbstor_t *dbs, dbstor_lookup_t *lu, int len, fixarray_t **fa);

#endif

