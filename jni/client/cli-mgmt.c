#include "cli-mgmt.h"

#include "iarch_mgmt.h"
#include "fixarray.h"
#include "session.h"
#include "md5.h"

#include <unistd.h>


cli_request(seed, im_seed_req, im_seed_rsp, PROTO_CMD_SETUP)
cli_request(auth, im_auth_req,im_auth_rsp, PROTO_CMD_SETUP)

cli_request(user_add, im_user_add_req, im_user_add_rsp,PROTO_CMD_USER_ADD)
cli_request(user_del, im_user_del_req, im_user_del_rsp, PROTO_CMD_USER_DEL)
cli_request(user_list, im_user_list_req, im_user_list_rsp, PROTO_CMD_USER_LIST)

cli_request(system_status, im_system_status_req, im_system_status_rsp, PROTO_CMD_SYSTEM_STATUS)

int client_setup(client_t *cli, char *user, char *where, char *cred, int cred_len){
    im_seed_req_t  sreq;
    im_seed_rsp_t  *srsp;
    im_auth_req_t  areq;
    im_auth_rsp_t  *arsp;
    int rc = 0;
    md5_state_t md;
    md5_byte_t digest[16];

    if(cli->status < CLIENT_STATUS_INIT){
        return -EINVAL;
    }

    // exchange seed information
    sreq.user = user;
    sreq.seed_len = 0;
    sreq.seed = NULL;

    rc = cli_request_seed(cli->ses, &sreq, &srsp);
    if(rc != 0){
        return rc;
    }

    // md5
    md5_init(&md);
    md5_append(&md, (md5_byte_t*)cred, cred_len);
    md5_append(&md, (md5_byte_t*)srsp->seed, srsp->seed_len);
    md5_finish(&md, digest);

    // authen user
    areq.user = user;
    areq.where = where;
    areq.cred_len = 16;
    areq.cred = (void*)digest;
    rc = cli_request_auth(cli->ses, &areq, &arsp);
    if(rc != 0){
        return rc;
    }

    cli->status = CLIENT_STATUS_AUTH;
    im_auth_rsp_free(arsp);

    return rc;
}

int client_close(client_t *cli){
    
    session_close(cli->ses);
    
    client_fini(cli);

    return 0;
}

int client_user_add(client_t *cli, char *user, char *cred, int type){
    im_user_add_req_t req;
    im_user_add_rsp_t *rsp;
    int rc;

    req.user = user;
    req.cred_len = strlen(cred);
    req.cred = cred;
    req.type = type;

    rc = cli_request_user_add(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    im_user_add_rsp_free(rsp);

    return rc;
}

int client_user_del(client_t *cli, char *user){
    im_user_del_req_t req;
    im_user_del_rsp_t *rsp;
    int rc;

    req.user = user;

    rc = cli_request_user_del(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    im_user_del_rsp_free(rsp);

    return rc;
}

int client_user_list(client_t *cli, int type, fixarray_t **users){
    im_user_list_req_t req;
    im_user_list_rsp_t *rsp;
    int rc;

    req.type =  type;

    rc = cli_request_user_list(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    *users = rsp->users;
    rsp->users = NULL;

    im_user_list_rsp_free(rsp);

    return rc;
}

int client_system_status(client_t *cli, cli_system_status_t **sta){
    im_system_status_req_t req;
    im_system_status_rsp_t *rsp;
    int rc;

    req.fake =  0;

    rc = cli_request_system_status(cli->ses, &req, &rsp);
    if(rc != 0){
        return rc;
    }

    *sta = rsp;

    return rc;
}

