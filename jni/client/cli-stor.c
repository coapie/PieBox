
#include "cli-stor.h"

#include "proto.h"
#include "iarch_stor.h"
#include "fixarray.h"
#include "session.h"
#include "cli-trans.h"

cli_request(lookup_create, stor_lookup_create_req, stor_lookup_create_rsp, PROTO_CMD_LOOKUP_CREATE)
cli_request(lookup_destroy, stor_lookup_destroy_req, stor_lookup_destroy_rsp, PROTO_CMD_LOOKUP_DESTROY)
cli_request(lookup_next, stor_lookup_next_req, stor_lookup_next_rsp, PROTO_CMD_LOOKUP_NEXT)

cli_request(fstat, stor_fstat_req, stor_fstat_rsp, PROTO_CMD_FSTAT)
cli_request(fopen, stor_fopen_req, stor_fopen_rsp, PROTO_CMD_FOPEN)
cli_request(fclose, stor_fclose_req, stor_fclose_rsp, PROTO_CMD_FCLOSE)
cli_request(fread, stor_fread_req, stor_fread_rsp, PROTO_CMD_FREAD)
cli_request(fwrite, stor_fwrite_req, stor_fwrite_rsp, PROTO_CMD_FWRITE)
cli_request(ftruncate, stor_ftruncate_req, stor_ftruncate_rsp, PROTO_CMD_FTRUNCATE)

cli_request(fcreate, stor_fcreate_req, stor_fcreate_rsp, PROTO_CMD_FCREATE)
cli_request(fdelete, stor_fdelete_req, stor_fdelete_rsp, PROTO_CMD_FDELETE)
cli_request(fmoveto, stor_fmoveto_req, stor_fmoveto_rsp, PROTO_CMD_FMOVETO)

int64_t clis_lookup_create(client_t *cli, int64_t folder, int type, int asc){
    stor_lookup_create_req_t req;
    stor_lookup_create_rsp_t *rsp;
    int64_t luid;
    int rc;

    req.folder = folder;
    req.type = type;
    req.asc = asc;
    
    rc = cli_request_lookup_create(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    luid = rsp->luid;

    stor_lookup_create_rsp_free(rsp);

    return luid;
}

int clis_lookup_destroy(client_t *cli, int64_t luid){
    stor_lookup_destroy_req_t req;
    stor_lookup_destroy_rsp_t *rsp;
    int rc;

    req.luid = luid;
    
    rc = cli_request_lookup_destroy(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_lookup_destroy_rsp_free(rsp);

    return rc;
}

int clis_lookup_next(client_t *cli, int64_t luid, int num, fixarray_t **items){
    stor_lookup_next_req_t req;
    stor_lookup_next_rsp_t *rsp;
    int rc;

    req.luid = luid;
    req.num  = num;

    rc = cli_request_lookup_next(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    *items = rsp->rfiles;
    rsp->rfiles = NULL;

    stor_lookup_next_rsp_free(rsp);

    return rc;
}

int clis_file_stat(client_t *cli, int64_t fid, rfile_t **rf){
    stor_fstat_req_t req;
    stor_fstat_rsp_t *rsp;
    int rc;

    req.fid = fid;
    
    rc = cli_request_fstat(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    *rf = (rfile_t *)rsp;

    return 0;

}

int clis_file_open(client_t *cli, int64_t fid){
    stor_fopen_req_t req;
    stor_fopen_rsp_t *rsp;
    int rc;

    req.fid = fid;
    
    rc = cli_request_fopen(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_fopen_rsp_free(rsp);

    return rc;
}

int clis_file_close(client_t *cli, int64_t fid){
    stor_fclose_req_t req;
    stor_fclose_rsp_t *rsp;
    int rc;

    req.fid = fid;
    
    rc = cli_request_fclose(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_fclose_rsp_free(rsp);

    return rc;
}

int clis_file_truncate(client_t *cli, int64_t fid, int64_t len){
    stor_ftruncate_req_t req;
    stor_ftruncate_rsp_t *rsp;
    int rc;

    req.fid = fid;
    req.len = len;
    
    rc = cli_request_ftruncate(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_ftruncate_rsp_free(rsp);

    return rc;
}

int clis_file_read(client_t *cli, int64_t fid, int64_t offset, void *buf, int len){
    stor_fread_req_t req;
    stor_fread_rsp_t *rsp;
    int rc;

    req.fid = fid;
    req.offset = offset;
    req.len = len;
    
    rc = cli_request_fread(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    // FIXME : double copy data
    len = rsp->data_len;
    memcpy(buf, rsp->data, len);
    stor_fread_rsp_free(rsp);

    return len;
}

int clis_file_write(client_t *cli, int64_t fid, int64_t offset, void *buf, int len){
    stor_fwrite_req_t req;
    stor_fwrite_rsp_t *rsp;
    int rc;

    req.fid = fid;
    req.offset = offset;
    req.data_len = len;
    req.data = buf;
    
    rc = cli_request_fwrite(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    rc = rsp->len;
    stor_fwrite_rsp_free(rsp);

    return rc;
}

int64_t clis_file_create(client_t *cli, char *file, int type, int64_t folder){
    stor_fcreate_req_t req;
    stor_fcreate_rsp_t *rsp;
    int64_t fid;
    int rc;

    req.fname = file;
    req.type = type;
    req.folder = folder;
    
    rc = cli_request_fcreate(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    fid = rsp->fid;
    stor_fcreate_rsp_free(rsp);

    return fid;
}

int clis_file_delete(client_t *cli, int64_t fid){
    stor_fdelete_req_t req;
    stor_fdelete_rsp_t *rsp;
    int rc;

    req.fid = fid;
    
    rc = cli_request_fdelete(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_fdelete_rsp_free(rsp);

    return rc;
}

int clis_file_moveto(client_t *cli, int64_t fid, int64_t folder, char *new_name){
    stor_fmoveto_req_t req;
    stor_fmoveto_rsp_t *rsp;
    int rc;

    req.fid = fid;
    req.folder = folder;
    req.fname = new_name;
    
    rc = cli_request_fmoveto(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    stor_fmoveto_rsp_free(rsp);

    return rc;
}

