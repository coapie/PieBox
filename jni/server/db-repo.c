#include "db-repo.h"
#include "proto.h"

#include "osapi.h"
#include "logger.h"
#include "sqlite3.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/limits.h>

#define SUBDIR_LEN        8

static const char dbrepo_default_basedir[][SUBDIR_LEN] = {
    "/video/",
    "/audio/",
    "/photo/",
    "/docs/",
    ""
};

int dbrepo_init(dbrepo_t *repo, const char *rdir, const char *dbstor, const char *dbuser){
    char *db;
    int len;
    int i;
    int rc;

    assert((repo != NULL) && (rdir != NULL) && (dbstor != NULL) && (dbuser != NULL));

    repo->rdir = strdup(rdir);
    if(repo->rdir == NULL){
        log_warn("init repo rdir or rdb fail!\n");
        return -1;
    }

    len = strlen(rdir) + strlen(dbuser) + 2;
    db = osapi_malloc(len);
    if(db == NULL){
        return -1;
    }
    strcpy(db, rdir);
    strcat(db, dbuser);
    rc =  dbuser_init(&repo->dbuser, db);
    if(rc != 0){
        free(repo->rdir);
        log_warn("init dbuser fail\n");
        return -1;
    }
    osapi_free(db);

    len = strlen(rdir) + strlen(dbstor) + 2;
    db = osapi_malloc(len);
    if(db == NULL){
        return -1;
    }
    strcpy(db, rdir);
    strcat(db, dbstor);
    if(dbstor_init(&repo->dbstor, db) != 0){
        log_warn("repo init open rdb fail!\n");
        free(repo->rdir);
        dbuser_fini(&repo->dbuser);
        return -1;
    }
    osapi_free(db);
    
    len = strlen(rdir) + SUBDIR_LEN;
    db = osapi_malloc(len);
    if(db == NULL){
        return -1;
    }
    for(i = 0; i < 4; i++){
        strcpy(db, rdir);
        strcat(db, dbrepo_default_basedir[i]);
        mkdir(db, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }
    osapi_free(db);

    fixmap_init(&repo->files);
    pthread_mutex_init(&repo->mlock, NULL);

    return 0;
}

int dbrepo_rfile_insert(dbrepo_t *repo, int64_t key, rfile_t *file){
    int rc;

    file->rnode.key = key;
    pthread_mutex_lock(&repo->mlock);
    rc = fixmap_insert(&repo->files, key, &file->rnode);
    pthread_mutex_unlock(&repo->mlock);

    return rc;
}

int dbrepo_rfile_lookup(dbrepo_t *repo, int64_t key, rfile_t **file){
    fixmap_node_t *node = NULL;
    int rc;

    pthread_mutex_lock(&repo->mlock);
    rc = fixmap_get(&repo->files, key, &node);
    pthread_mutex_unlock(&repo->mlock);

    if(node != NULL){
        *file = container_of(node, rfile_t, rnode);
    }

    return rc;
}

int dbrepo_rfile_remove(dbrepo_t *repo, int64_t key, rfile_t **file){
    fixmap_node_t *node = NULL;
    int rc;

    pthread_mutex_lock(&repo->mlock);
    rc = fixmap_remove(&repo->files, key, &node);
    pthread_mutex_unlock(&repo->mlock);

    if((node != NULL) && (file != NULL)){
        *file = container_of(node, rfile_t, rnode);
    }

    return rc;
}

int dbrepo_fini(dbrepo_t *repo){
    fixmap_node_t *fmn;
    struct rb_node *node;
    rfile_t *file;

    assert(repo != NULL);

    dbuser_fini(&repo->dbuser);
    dbstor_fini(&repo->dbstor);
    free(repo->rdir);

    pthread_mutex_destroy(&repo->mlock);

    fixmap_for_each((&repo->files.root), node, fmn){
        file = container_of(fmn, rfile_t, rnode);
        dbstor_file_free(file->file);
        fclose(file->handle);
    }

    return 0;
}

char *dbrepo_build_rpath(dbrepo_t *repo, int type, const char *fpath){
    char *rpath;
    int len;
    
    if(type > RFILE_TYPE_DOCS){
        return NULL;
    }

    len = strlen(repo->rdir) + strlen(fpath) + SUBDIR_LEN + 2;

    rpath = osapi_malloc(len);
    if(rpath == NULL){
        return NULL;
    }
    memset(rpath, 0, len);

    strcpy(rpath, repo->rdir);
    strcat(rpath, dbrepo_default_basedir[type]);
    strcat(rpath, fpath);

    return rpath;
}

int dbrepo_clean(dbrepo_t *repo){
    return 0;
}

