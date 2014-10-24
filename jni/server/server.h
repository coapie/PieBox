
#ifndef __SERVER_H__
#define __SERVER_H__

#include "list.h"
#include "fixmap.h"
#include "db-repo.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <netinet/in.h>

#include <pthread.h>

#define SERVER_DEFAULT_PORT     8086
#define SERVER_REPO_MAX         16

enum {
    SERVER_WORKER_CMD_ADDSESSION = 0,
    SERVER_WORKER_CMD_RMVSESSION,
    SERVER_WORKER_CMD_LISTSESSION,
    SERVER_WORKER_CMD_TERMINATE,
};

typedef struct _worker_cmd{
    unsigned int cmd;
    union {
        int sock;
        int retcode;
        void *data;
    };
}worker_cmd_t;

struct _worker;
struct _server;

typedef struct _session{
    struct _server *server;
    struct _worker *worker;
    struct bufferevent *bev;

    int sesid;

    ruser_t *user;

    fixmap_node_t node;
    fixmap_t fls;
    fixmap_t lus;
}session_t;

typedef struct _worker{
    // init by caller
    pthread_t wid;
    int sock_server;    // server -> worker
    int sock_worker;    // worker -> server
    int idx;    // index in workers
    struct _server *server;
    struct event *evserver; // wrap of sock_server

    // init by worker routine
    struct event_base *evbase;   // libevent base
    struct bufferevent *besock;   // bufferevent wrapped sock_worker
}worker_t;

typedef struct _server{
    struct event_base *evbase;

    int status;

    pthread_mutex_t lock;

    int worker_num;
    int worker_pos;
    worker_t *workers;

    int sesid;
    fixmap_t seses;

    fixmap_t users;

    dbrepo_t repo;
}server_t;

int server_init(server_t *svr, int workers);
int server_fini(server_t *svr);
int server_listen_on(server_t *svr, int sock);
int server_start(server_t *svr);

int session_rfile_insert(session_t *ses, int64_t fid, rfile_t *file);
int session_rfile_remove(session_t *ses, int64_t fid, rfile_t **file);
int session_rfile_find(session_t *ses, int64_t fid, rfile_t **file);

int session_lookup_insert(session_t *ses, int64_t luid, dbstor_lookup_t *lu);
int session_lookup_remove(session_t *ses, int64_t luid, dbstor_lookup_t **lu);
int session_lookup_find(session_t *ses, int64_t luid, dbstor_lookup_t **lu);

static inline int64_t session_uid(session_t *ses){
    // for test
    return 0;
    //    return ses->user->uid;
}

static inline char *session_where(session_t *ses){
    return "android";
//    return ses->user->from;
}

// implement in protocol.c
int request_dispatch(session_t *ses, struct evbuffer *in, struct evbuffer *out);

#endif // __SERVER_H__

