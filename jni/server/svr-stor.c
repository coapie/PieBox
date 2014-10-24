#include "svr-stor.h"

#include "db-repo.h"
#include "server.h"
#include "iarch_stor.h"
#include "osapi.h"
#include "logger.h"
 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int svr_fstat_rsp_from_dbfile(dbrepo_t *repo, dbstor_file_t *file, stor_fstat_rsp_t **fstat){
    stor_fstat_rsp_t *r;
    char *ch;
    char *rpath;
    struct stat sb;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fstat rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    r->fid = file->fid;
    r->fname = file->fname;
    file->fname = NULL;
    
    rpath = dbrepo_build_rpath(repo, file->ftype, file->fpath);
    if(rpath == NULL){
        stor_fstat_rsp_free(r);
        return -1;
    }

    // in fpath, before '/' is folder name
    r->folder = file->fpath;
    file->fpath = NULL;
    ch = strchr(r->folder, '/');
    if(ch != NULL){
        *ch = 0;
    }else{
        osapi_free(r->folder);
        r->folder = NULL;
    }

    r->type = file->ftype;
    r->snap_len = file->fsnap_len;
    r->snap = file->fsnap;
    file->fsnap_len = 0;
    file->fsnap = NULL;

    rc = stat(rpath, &sb);
    osapi_free(rpath);
    if(rc != 0){
        stor_fstat_rsp_free(r);
        return -1;
    }

    if(!(file->ftype&DBSTOR_FOLDER_TYPE_BIT)){
        r->fsize = (int64_t)sb.st_size;
    }else{
        r->fsize = 0;
    }

    r->fatime = (int64_t)sb.st_atime;   /* time of last access */
    r->fmtime = (int64_t)sb.st_mtime;   /* time of last modification */
    r->fctime = (int64_t)sb.st_ctime;   /* time of last status change */

    *fstat = r;

    return 0;
}

// file management operations
int svr_request_fstat_routine(session_t *ses, stor_fstat_req_t *req,
        stor_fstat_rsp_t **rsp){
    dbrepo_t *repo = &ses->server->repo;
    dbstor_file_t *file;
    int rc;

    rc = dbstor_flookup(&repo->dbstor, req->fid, &file);
    if(rc != 0){
        log_warn("file not exist in dbstor\n");
        return -1;
    }
    
    rc = svr_fstat_rsp_from_dbfile(repo, file, rsp);
    dbstor_file_free(file);
    if(rc != 0){
        log_warn("stat file fail\n");
        return -1;
    }
    
    return 0;
}

int svr_request_fopen_routine(session_t *ses, stor_fopen_req_t *req,
        stor_fopen_rsp_t **rsp){
    stor_fopen_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    dbstor_file_t *file;
    rfile_t *rf;
    char *rpath;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fopen rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    rc = dbrepo_rfile_lookup(repo, req->fid, &rf);
    if(rc == 0){
        *rsp = r;
        return 0;
    }
    
    rf = osapi_malloc(sizeof(*rf));
    if(rf == NULL){
        osapi_free(r);
        return -1;
    }

    rc = dbstor_flookup(&repo->dbstor, req->fid, &file);
    if(rc != 0){
        osapi_free(r);
        osapi_free(rf);
        return -ENOENT;
    }

    rf->fid = req->fid;
    rf->file = file;
    rpath = dbrepo_build_rpath(repo, file->ftype, file->fpath);
    rf->handle = fopen(rpath, "r+b");
    osapi_free(rpath);
    if(rf->handle == NULL){
        osapi_free(r);
        osapi_free(rf);
    }

    fixmap_node_init(&rf->rnode);
    fixmap_node_init(&rf->snode);
    dbrepo_rfile_insert(repo, req->fid, rf);
    session_rfile_insert(ses, req->fid, rf);

    *rsp = r;
    
    return 0;
}

int svr_request_fclose_routine(session_t *ses, stor_fclose_req_t *req,
        stor_fclose_rsp_t **rsp){
    stor_fclose_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    rfile_t *rf;
    int rc;

    rc = session_rfile_remove(ses, req->fid, &rf);
    if(rc != 0){
        return -1;
    }

    rc = dbrepo_rfile_remove(repo, req->fid, &rf);
    if(rc != 0){
        return -ENOENT;
    }

    fclose(rf->handle);
    dbstor_file_free(rf->file);
    osapi_free(rf);
    
    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fclose rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
    
    *rsp = r;
    
    return 0;
}

int svr_request_fread_routine(session_t *ses, stor_fread_req_t *req,
        stor_fread_rsp_t **rsp){
    stor_fread_rsp_t *r;
    rfile_t *rf;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    rc = session_rfile_find(ses, req->fid, &rf);
    if(rc != 0){
        log_warn("file not exist\n");
        osapi_free(r);
        return -ENOENT;
    }

    r->data = osapi_malloc(req->len);
    if(r->data == NULL){
        osapi_free(r);
        return -ENOMEM;
    }

    fseek(rf->handle, req->offset, SEEK_SET);
    r->data_len = fread(r->data, 1, req->len, rf->handle);
    if(r->data_len <= 0){
        osapi_free(r->data);
        osapi_free(r);
        return -EINVAL;
    }else{
        r->offset = req->offset;
        *rsp = r;
        return 0;
    }
}

int svr_request_fwrite_routine(session_t *ses, stor_fwrite_req_t *req,
        stor_fwrite_rsp_t **rsp){
    stor_fwrite_rsp_t *r;
    rfile_t *rf;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    rc = session_rfile_find(ses, req->fid, &rf);
    if(rc != 0){
        osapi_free(r);
        return -ENOENT;
    }

    fseek(rf->handle, req->offset, SEEK_SET);
   
    r->len = fwrite(req->data, 1, req->data_len, rf->handle);
    if(r->len <= 0){
        osapi_free(r);
        return -EINVAL;
    }else{
        r->offset = req->offset;
        r->fid = rf->fid;
        *rsp = r;
        return 0;
    }
}

int svr_request_ftruncate_routine(session_t *ses, stor_ftruncate_req_t *req,
        stor_ftruncate_rsp_t **rsp){
    stor_ftruncate_rsp_t *r;
    rfile_t *rf;
    int rc;

    rc = session_rfile_find(ses, req->fid, &rf);
    if(rc != 0){
        log_warn("file not open!\n");
        return -ENOENT;
    }

    rc = ftruncate(fileno(rf->handle), req->len);
    if(rc != 0){
        log_warn("truncate file fail:%d\n", rc);
        return -1;
    }

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
    
    *rsp = r;
    
    return 0;
}

int svr_request_fcreate_routine(session_t *ses, stor_fcreate_req_t *req,
        stor_fcreate_rsp_t **rsp){
    stor_fcreate_rsp_t *r;
    dbstor_file_t *file, *folder, df;
    dbrepo_t *repo = &ses->server->repo;
    char *rpath;
    FILE *handle;
    int rc;

    rc = dbstor_flookup_byname(&repo->dbstor, req->fname, req->folder, req->type, &file);
    if(rc == 0){
        log_warn("file already exist");
        dbstor_file_free(file);
        return -EEXIST;
    }

    rc = dbstor_flookup(&repo->dbstor, req->folder, &folder);
    if(rc != 0){
        log_warn("folder have not exist\n");
        return -EEXIST;
    }

    df.folder = folder->fid;
    df.fowner = session_uid(ses);
    df.ftype = req->type;

    df.fname = req->fname;
    df.fwhere = session_where(ses);

    // folder 0's fname is NULL
    df.fpath = osapi_malloc(((folder->fname != NULL)? strlen(folder->fname):0) + strlen(df.fname) + 2);
    if(df.fpath == NULL){
        return -ENOMEM;
    }

    if(folder->fname != NULL){
        strcpy(df.fpath, folder->fname);
        strcat(df.fpath, "/");
        strcat(df.fpath, df.fname);
    }else{
        strcpy(df.fpath, df.fname);
    }

    df.fsnap_len = 0;
    df.fsnap = NULL;

    dbstor_file_free(folder);

    rc = dbstor_finsert(&repo->dbstor, &df);
    if(rc != 0){
        osapi_free(df.fpath);
        log_warn("delete file in db fail\n");
        return -1;
    }

    rpath = dbrepo_build_rpath(repo, df.ftype, df.fpath);
    if(rpath == NULL){
        osapi_free(df.fpath);
        return -1;
    }

    handle = fopen(rpath,"wb+");
    osapi_free(df.fpath);
    osapi_free(rpath);
    if(handle == NULL){
        log_warn("create file fail\n");
        return -1;
    }
    fclose(handle);

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
    r->fid = df.fid;
 
    *rsp = r;
    
    return 0;
}

int svr_request_fdelete_routine(session_t *ses, stor_fdelete_req_t *req,
        stor_fdelete_rsp_t **rsp){
    stor_fdelete_rsp_t *r;
    dbstor_file_t *file;
    dbrepo_t *repo = &ses->server->repo;
    rfile_t *rf;
    char *rpath;
    int rc;

    rc = dbrepo_rfile_lookup(repo, req->fid, &rf);
    if(rc == 0){
        log_warn("file still opened\n");
        return -EEXIST;
    }

    rc = dbstor_flookup(&repo->dbstor, req->fid, &file);
    if(rc != 0){
        log_warn("file not in db\n");
        return -ENOENT;
    }

    rc = dbstor_fdelete(&repo->dbstor, req->fid);
     if(rc != 0){
        log_warn("delete file in db fail\n");
        dbstor_file_free(file);
        return -1;
    }

    rpath = dbrepo_build_rpath(repo, file->ftype, file->fpath);
    if(rpath == NULL){
        log_warn("build real path fail\n");
        dbstor_file_free(file);
        return -1;
    }

    dbstor_file_free(file);

    rc = remove(rpath);
    osapi_free(rpath);
    if(rc != 0){
        log_warn("remove file fail!\n");
        return -1;
    }

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("already remove file but return wrong code\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
 
    *rsp = r;
    
    return 0;
}

int svr_request_fmoveto_routine(session_t *ses, stor_fmoveto_req_t *req,
        stor_fmoveto_rsp_t **rsp){
    stor_fmoveto_rsp_t *r;
    dbstor_file_t *file, *folder, df;
    dbrepo_t *repo = &ses->server->repo;
    rfile_t *rf;
    char *rpath_src, *rpath_dst;
    int len;
    int rc;

    rc = dbrepo_rfile_lookup(repo, req->fid, &rf);
    if(rc == 0){
        log_warn("file still opened\n");
        return -EEXIST;
    }

    rc = dbstor_flookup(&repo->dbstor, req->fid, &file);
    if(rc != 0){
        log_warn("file not in db\n");
        return -ENOENT;
    }

    rc = dbstor_flookup(&repo->dbstor, req->folder, &folder);
    if(rc != 0){
        log_warn("dst folder not exist\n");
        dbstor_file_free(file);
        return -ENOENT;
    }

    df.fid = file->fid;
    df.folder = req->folder;
    df.fowner = file->fowner;
    df.ftype = file->ftype;

    df.fname = file->fname;
    df.fwhere = file->fwhere;

    len = strlen(file->fname) + strlen(folder->fname) + 2;
    df.fpath = osapi_malloc(len);
    if(df.fpath == NULL){
        dbstor_file_free(file);
        dbstor_file_free(folder);
        return -ENOMEM;
    }
    strcpy(df.fpath, folder->fname);
    strcat(df.fpath, "/");
    strcat(df.fpath, file->fname);

    dbstor_file_free(folder);

    df.fsnap_len = file->fsnap_len;
    df.fsnap = file->fsnap;
    
    rpath_src = dbrepo_build_rpath(repo, file->ftype, file->fpath);
    if(rpath_src == NULL){
        osapi_free(df.fpath);
        dbstor_file_free(file);
        return -1;
    }

    rpath_dst = dbrepo_build_rpath(repo, df.ftype, df.fpath);
    if(rpath_dst == NULL){
        osapi_free(df.fpath);
        dbstor_file_free(file);
        osapi_free(rpath_src);
        return -1;
    }

    rc = rename(rpath_src, rpath_dst);
    if(rc != 0){
        log_warn("rename file fail\n");
        osapi_free(df.fpath);
        dbstor_file_free(file);
        osapi_free(rpath_src);
        osapi_free(rpath_dst);
        return -1;
    }

    rc = dbstor_fupdate(&repo->dbstor, file->fid, &df);
    osapi_free(df.fpath);
    if(rc != 0){
        rename(rpath_dst, rpath_src);
        dbstor_file_free(file);
        osapi_free(rpath_src);
        osapi_free(rpath_dst);
        log_warn("update db fail!\n");
        return -1;
    }

    osapi_free(rpath_src);
    osapi_free(rpath_dst);
    dbstor_file_free(file);

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
 
    *rsp = r;
    
    return 0;
}

int svr_request_lookup_create_routine(session_t *ses, stor_lookup_create_req_t *req,
        stor_lookup_create_rsp_t **rsp){
    stor_lookup_create_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    dbstor_lookup_t *lu;
    int rc;

    rc = dbstor_lucreate(&repo->dbstor, req->folder, req->type, req->asc, &lu);
    if(rc != 0){
        log_warn("create lookup fail\n");
        return -1;
    }

    session_lookup_insert(ses, lu->luid, lu);

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
 
    r->luid = lu->luid;
    *rsp = r;
    
    return 0;
}

int svr_request_lookup_destroy_routine(session_t *ses, stor_lookup_destroy_req_t *req,
        stor_lookup_destroy_rsp_t **rsp){
    stor_lookup_destroy_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    dbstor_lookup_t *lu;
    int rc;

    rc = session_lookup_remove(ses, req->luid, &lu);
    if(rc != 0){
        log_warn("lookup not exist\n");
        return -ENOENT;
    }

    dbstor_ludestroy(&repo->dbstor, lu);

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));
 
    *rsp = r;
    
    return 0;
}

int svr_request_lookup_next_routine(session_t *ses, stor_lookup_next_req_t *req,
        stor_lookup_next_rsp_t **rsp){
    stor_lookup_next_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    dbstor_lookup_t *lu;
    dbstor_file_t *file;
    stor_fstat_rsp_t *fstat;
    fixarray_t *fa;
    int i;
    int rc;

    rc = session_lookup_find(ses, req->luid, &lu);
    if(rc != 0){
        log_warn("lookup not exist\n");
        return -ENOENT;
    }

    rc = dbstor_lunext(&repo->dbstor, lu, req->num, &fa);
    if(rc != 0){
        log_warn("lookup call lunext fail\n");

        for(i = 0; i < fixarray_num(fa); i++){
            fixarray_get(fa, i, (void **)&file);
            dbstor_file_free(file);
        }
        fixarray_destroy(fa);

        return -1;
    }

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for fread rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    for(i = 0; i < fixarray_num(fa); i++){
        fixarray_get(fa, i, (void**)&file);
        
        fstat = NULL;
        svr_fstat_rsp_from_dbfile(repo, file, &fstat);
        fixarray_set(fa, i, fstat);

        dbstor_file_free(file);
    }

    r->rfiles = fa;
    *rsp = r;
    
    return 0;
}

