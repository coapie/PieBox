
#include "svr-mgmt.h"
#include "db-user.h"
#include "osapi.h"
#include "logger.h"
#include "iarch_mgmt.h"

#include "server.h"
#include "md5.h"

int svr_request_seed_routine(session_t *ses, im_seed_req_t *req, im_seed_rsp_t **rsp){
    im_seed_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    ruser_t *ru;
    int64_t uid;
    md5_state_t mds;

    if(ses->user == NULL){
        uid = dbuser_lookup(&repo->dbuser, req->user, &ru);
        if(uid < 0){
            log_info("seed user:%s not exist\n", req->user);
            return -ENOENT;
        }
        ses->user = ru;

        snprintf((char*)ru->seed, DBUSER_CRED_LEN, "%ld", random());
        md5_init(&mds);
        md5_append(&mds, ru->seed, DBUSER_CRED_LEN);
        md5_finish(&mds, ru->seed);
    }

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for auth rsp\n");
        return -1;
    }
    memset(r, 0, sizeof(*r));

    r->sesid = ses->sesid;
    r->user = req->user;
    req->user = NULL;
    r->seed_len = DBUSER_CRED_LEN;
    memcpy(r->seed, ses->user->seed, DBUSER_CRED_LEN);

    *rsp = r;

    return 0;
}

int svr_request_auth_routine(session_t *ses, im_auth_req_t *req, im_auth_rsp_t **rsp){
    im_auth_rsp_t *r;
    md5_state_t mds;
    md5_byte_t cred[16];

    if(ses->user == NULL){
        log_warn("seed first\n");
        return -EINVAL;
    }

    if(req->cred_len != 16){
        log_warn("cred size is wrong!\n");
        return -EINVAL;
    }

    if(strcmp(ses->user->user, req->user) != 0){
        log_warn("wrong user name\n");
        return -EINVAL;
    }
        
    md5_init(&mds);
    md5_append(&mds, ses->user->seed, DBUSER_CRED_LEN);
    md5_append(&mds, req->cred, req->cred_len);
    md5_finish(&mds, cred);

    if(memcmp(cred, req->cred, 16) != 0){
        log_warn("passwd fail\n");
        return -EINVAL;
    }
    strncpy(ses->user->from, req->where, DBUSER_FROM_LEN);
    ses->user->from[DBUSER_FROM_LEN-1] = 0;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for auth rsp\n");
        return -1;
    }
    memset(r, 0, sizeof(*r));

    r->uid = ses->user->uid;

    *rsp = r;
    return 0;
}

int svr_request_user_add_routine(session_t *ses, im_user_add_req_t *req,
        im_user_add_rsp_t **rsp){
    im_user_add_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for add rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    ((unsigned char *)req->cred)[req->cred_len] = 0;
    r->uid = dbuser_add(&repo->dbuser, req->user, (unsigned char *)req->cred, req->type);
    if(r->uid < 0){
        osapi_free(r);
        return (int)r->uid;
    }

    *rsp = r;

    return 0;
}

int svr_request_user_del_routine(session_t *ses, im_user_del_req_t *req,
        im_user_del_rsp_t **rsp){
    im_user_del_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for del rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    r->uid = dbuser_del(&repo->dbuser, req->user);
    if(r->uid < 0){
        osapi_free(r);
        return (int)r->uid;
    }

    *rsp = r;

    return 0;
}

int svr_request_user_list_routine(session_t *ses, im_user_list_req_t *req,
        im_user_list_rsp_t **rsp){
    im_user_list_rsp_t *r;
    fixarray_t *users;
    dbrepo_t *repo = &ses->server->repo;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for list rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    rc = dbuser_list(&repo->dbuser, &users);
    if(rc != 0){
        osapi_free(r);
        return rc;
    }

    r->users = users;
    *rsp = r;

    return 0;
}

int svr_request_user_lookup_routine(session_t *ses, im_user_lookup_req_t *req,
        im_user_lookup_rsp_t **rsp){
    im_user_lookup_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    ruser_t *u;
    int rc;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for lookup rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    rc = dbuser_lookup(&repo->dbuser, req->user, &u);
    if(rc != 0){
        osapi_free(r);
        return rc;
    }

    r->uid = u->uid;
    r->type = u->type;

    osapi_free(u);
    *rsp = r;

    return 0;
}

int svr_request_user_update_routine(session_t *ses, im_user_update_req_t *req,
        im_user_update_rsp_t **rsp){
    im_user_update_rsp_t *r;
    dbrepo_t *repo = &ses->server->repo;
    int64_t uid;

    r = osapi_malloc(sizeof(*r));
    if(r == NULL){
        log_warn("no memory for update rsp\n");
        return -ENOMEM;
    }
    memset(r, 0, sizeof(*r));

    ((unsigned char *)req->cred)[req->cred_len] = 0;
    uid = dbuser_update(&repo->dbuser, req->user, (unsigned char*)req->cred, req->type);
    if(uid < 0){
        osapi_free(r);
        return -1;
    }

    r->errcode = 0;

    *rsp = r;

    return 0;
}

int svr_request_system_status_routine(session_t *ses, im_system_status_req_t *req,
        im_system_status_rsp_t **rsp){
    // not 
    return -1;
}

