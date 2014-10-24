
#include "svr-trans.h"

#include "proto.h"
#include "server.h"

#include "osapi.h"
#include "logger.h"

#include "fixbuffer.h"
#include "fixarray.h"

#include "iarch_mgmt.h"
#include "iarch_stor.h"

#include <event2/buffer.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <assert.h>
#include <errno.h>

static int header_unpack(struct evbuffer *in, proto_header_t **buf){
    proto_header_t   *pro;
    int16_t   nsize = PROTO_HEADER_LEN;
    int       len;
    int evlen;
    ev_ssize_t es;
    
    evlen = evbuffer_get_length(in);

    if(evlen == 0){
        return -EINVAL;
    }
    
    if(evlen < PROTO_HEADER_LEN){
        log_warn("evbuffer - buffer is to small:%d\n", evlen);
        return -EINVAL;
    }

    // get the payload size
    es = evbuffer_copyout(in, &nsize, sizeof(nsize));
    if(es != sizeof(nsize)){
        log_warn("evbuffer read command size fail!\n");
        return -EINVAL;
    }
    len = ntohs(nsize) + PROTO_HEADER_LEN;

    evlen = evbuffer_get_length(in);
    if(evlen < len){
//      log_warn("evbuffer - buffer is less than on package:%d\n", evlen);
        return -EINVAL;
    }

    pro = osapi_malloc(len);
    if(pro == NULL){
        log_warn("no enough memory for unpack protocols\n");
        return -ENOMEM;
    }
    memset(pro, 0, len);

    es = evbuffer_copyout(in, pro, len);
    if(es != len){
        log_warn("read whole package from buffer fail:%d", es);
        osapi_free(pro);
        return -EINVAL;
    }
    evbuffer_drain(in, len);
  
    pro->size = ntohs(pro->size);

    pro->sesid = ntohl(pro->sesid);
    pro->seqid = ntohl(pro->seqid);
    pro->cred = ntohl(pro->cred);

    *buf = pro;

    return 0;
}

static int header_pack(proto_header_t *in){

    assert(in != NULL);

    in->size  = htons(in->size);

    in->sesid = htonl(in->sesid);
    in->seqid = htonl(in->seqid);   
    in->cred  = htonl(in->cred);

    return 0;
}

static inline int trans_ack_data(struct evbuffer *out, proto_header_t *pro, struct evbuffer *buf){
    assert((out != NULL) && (pro != NULL));

    if(buf != NULL){
        pro->size = evbuffer_get_length(buf);
    }else{
        pro->size = 0;
    }

    header_pack(pro);

    evbuffer_add(out, pro, sizeof(*pro));

    if(buf != NULL)
        evbuffer_add_buffer(out, buf);

    return 0;
}

static int trans_ack_fail(struct evbuffer *out, proto_header_t *pro, uint8_t errcode){
    int rc;

    assert((out != NULL) && (pro != NULL));

    pro->errcode = errcode;
    pro->size = 0;

    rc = trans_ack_data(out, pro, NULL);

    return rc;
}

static bool fb_reader(struct cmp_ctx_s *ctx, void *data, size_t limit){
    fixbuffer_t *fb = (fixbuffer_t *)ctx->buf;
    size_t copy;

    copy = fixbuffer_copy(fb, data, limit);
    if(copy == limit){
        fixbuffer_drain(fb, copy);
        return true;
    }

    return false;
}

static size_t fb_writer(struct cmp_ctx_s *ctx, const void *data, size_t count){
    fixbuffer_t *fb = (fixbuffer_t *)ctx->buf;

    return fixbuffer_write(fb, data, count);
}

static bool evb_reader(struct cmp_ctx_s *ctx, void *data, size_t limit){
    struct evbuffer *evb = (struct evbuffer *)ctx->buf;
    size_t len;

    len = evbuffer_get_length(evb);
    if(limit > len)
        return false;

    evbuffer_remove(evb, data, limit);
    return true;
}

static size_t evb_writer(struct cmp_ctx_s *ctx, const void *data, size_t count){
    struct evbuffer *evb = (struct evbuffer *)ctx->buf;
    
    if(evbuffer_add(evb, data, count) == 0)
        return count;
    else
        return 0;
}

// auth operations
svr_request(seed, im_seed_req, im_seed_rsp)
svr_request(auth, im_auth_req, im_auth_rsp)

// user operations
svr_request(user_add, im_user_del_req, im_user_add_rsp)
svr_request(user_del, im_user_del_req, im_user_del_rsp)
svr_request(user_list, im_user_list_req, im_user_list_rsp)
svr_request(user_update, im_user_update_req, im_user_update_rsp)
svr_request(user_lookup, im_user_lookup_req, im_user_lookup_rsp)

// system management operations
svr_request(system_status, im_system_status_req, im_system_status_rsp)

// file management operations
svr_request(fstat, stor_fstat_req, stor_fstat_rsp)
svr_request(fopen, stor_fopen_req, stor_fopen_rsp)
svr_request(fclose, stor_fclose_req, stor_fclose_rsp)
svr_request(fread, stor_fread_req, stor_fread_rsp)
svr_request(fwrite, stor_fwrite_req, stor_fwrite_rsp)
svr_request(ftruncate, stor_ftruncate_req, stor_ftruncate_rsp)

svr_request(fcreate, stor_fcreate_req, stor_fcreate_rsp)
svr_request(fdelete, stor_fdelete_req, stor_fdelete_rsp)
svr_request(fmoveto, stor_fmoveto_req, stor_fmoveto_rsp)

svr_request(lookup_create, stor_lookup_create_req, stor_lookup_create_rsp)
svr_request(lookup_destroy, stor_lookup_destroy_req, stor_lookup_destroy_rsp)
svr_request(lookup_next, stor_lookup_next_req, stor_lookup_next_rsp)

int request_dispatch(session_t *ses, struct evbuffer *in, struct evbuffer *out){
    proto_header_t *pro;

    assert((ses != NULL) && (in != NULL) && (out != NULL));

    while(header_unpack(in, &pro) == 0){
        switch(pro->cmdcode){
        case PROTO_CMD_SEED:
            svr_request_seed(ses, in, out, pro);
            break;

        case PROTO_CMD_SETUP:
            svr_request_auth(ses, in, out, pro);
            break;

        case PROTO_CMD_USER_ADD:
            svr_request_user_add(ses, in, out, pro);
            break;

        case PROTO_CMD_USER_DEL:
            svr_request_user_del(ses, in, out, pro);
            break;

        case PROTO_CMD_USER_LIST:
            svr_request_user_list(ses, in, out, pro);
            break;

        case PROTO_CMD_USER_UPDATE:
            svr_request_user_update(ses, in, out, pro);
            break;

        case PROTO_CMD_USER_LOOKUP:
            svr_request_user_lookup(ses, in, out, pro);
            break;

        case PROTO_CMD_SYSTEM_STATUS:
            svr_request_system_status(ses, in, out, pro);
            break;

        case PROTO_CMD_FOPEN:
            svr_request_fopen(ses, in, out, pro);
            break;

        case PROTO_CMD_FCLOSE:
            svr_request_fclose(ses, in, out, pro);
            break;

        case PROTO_CMD_FWRITE:
            svr_request_fwrite(ses, in, out, pro);
            break;

        case PROTO_CMD_FREAD:
            svr_request_fread(ses, in, out, pro);
            break;
    
        case PROTO_CMD_FSTAT:
            svr_request_fstat(ses, in, out, pro);
            break;

        case PROTO_CMD_FTRUNCATE:
            svr_request_ftruncate(ses, in, out, pro);
            break;

        case PROTO_CMD_FMOVETO:
            svr_request_fmoveto(ses, in, out, pro);
            break;

        case PROTO_CMD_FCREATE:
            svr_request_fcreate(ses, in, out, pro);
            break;

        case PROTO_CMD_FDELETE:
            svr_request_fdelete(ses, in, out, pro);
            break;

        case PROTO_CMD_LOOKUP_CREATE:
            svr_request_lookup_create(ses, in, out, pro);
            break;

        case PROTO_CMD_LOOKUP_DESTROY:
            svr_request_lookup_destroy(ses, in, out, pro);
            break;

        case PROTO_CMD_LOOKUP_NEXT:
            svr_request_lookup_next(ses, in, out, pro);
            break;

        default:
            log_warn("received command is unkown:%d\n", pro->cmdcode);
            trans_ack_fail(out, pro, PROTO_RSP_ECODE_INVALID);
            break;
        }

        pro = NULL;
    }

    return 0;
}
    
