
#ifndef __DBREPO_H__
#define __DBREPO_H__

#include "db-user.h"
#include "db-stor.h"
#include "fixarray.h"
#include "sqlite3.h"

#include <stdio.h>
#include <pthread.h>

/*
** repository
*/
typedef struct  _dbrepo{
    char    *rdir;
    char    *rdb_fname;

    pthread_mutex_t mlock;
    fixmap_t files;

    dbstor_t dbstor;
    dbuser_t dbuser;
}dbrepo_t;

typedef struct _dbrepo_file{
    fixmap_node_t snode;
    fixmap_node_t rnode;

    int64_t fid;
    FILE *handle;
    dbstor_file_t *file;
}rfile_t;

int dbrepo_init(dbrepo_t *repo, const char *rdir, const char *rdbs, const char *rdbu);
int dbrepo_fini(dbrepo_t *repo);

char *dbrepo_build_rpath(dbrepo_t *repo, int type, const char *fpath);

int dbrepo_rfile_insert(dbrepo_t *repo, int64_t key, rfile_t *file);
int dbrepo_rfile_lookup(dbrepo_t *repo, int64_t key, rfile_t **file);
int dbrepo_rfile_remove(dbrepo_t *repo, int64_t key, rfile_t **file);

#endif

