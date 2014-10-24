#include "server.h"
#include "osapi.h"
#include "logger.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define WORKER_DEFAULT_HSET_LEN         4

int session_rfile_insert(session_t *ses, int64_t fid, rfile_t *file){
    return fixmap_insert(&ses->fls, fid, &file->snode);
}

int session_rfile_remove(session_t *ses, int64_t fid, rfile_t **file){
    fixmap_node_t *node = NULL;
    int rc;

    rc = fixmap_remove(&ses->fls, fid, &node);
    if((node != NULL) && (file != NULL))
        *file = container_of(node, rfile_t, snode);

    return rc;
}

int session_rfile_find(session_t *ses, int64_t fid, rfile_t **file){
    fixmap_node_t *node = NULL;
    int rc;

    rc = fixmap_get(&ses->fls, fid, &node);
    if(node != NULL)
        *file = container_of(node, rfile_t, snode);

    return rc;
}

int session_lookup_insert(session_t *ses, int64_t luid, dbstor_lookup_t *lu){
    return fixmap_insert(&ses->lus, luid, &lu->node);
}

int session_lookup_remove(session_t *ses, int64_t luid, dbstor_lookup_t **lu){
    fixmap_node_t *node = NULL;
    int rc;

    rc = fixmap_remove(&ses->lus, luid, &node);
    if((node != NULL) && (lu != NULL))
        *lu = container_of(node, dbstor_lookup_t, node);

    return rc;
}

int session_lookup_find(session_t *ses, int64_t luid, dbstor_lookup_t **lu){
    fixmap_node_t *node = NULL;
    int rc;

    rc = fixmap_get(&ses->lus, luid, &node);
    if((node != NULL) && (lu != NULL))
        *lu = container_of(node, dbstor_lookup_t, node);

    return rc;
}

static void session_readcb(struct bufferevent *bev, void *ctx){
    session_t   *ses = ctx;
    struct evbuffer *in, *out;

    assert((bev != NULL) && (ctx != NULL));

    in = bufferevent_get_input(bev);
    out = bufferevent_get_output(bev);

    request_dispatch(ses, in, out);
}

static void session_writecb(struct bufferevent *bev, void *ctx){
}

static void session_destroy(session_t *ses){
    fixmap_node_t *fmn;
    struct rb_node *node;
    rfile_t *file;
    dbstor_lookup_t *lu;

    bufferevent_disable(ses->bev, EV_READ|EV_WRITE);
    bufferevent_free(ses->bev);

    pthread_mutex_lock(&ses->server->lock);
    fixmap_remove(&ses->server->seses, ses->sesid, NULL);
    pthread_mutex_unlock(&ses->server->lock);

    pthread_mutex_lock(&ses->server->repo.mlock);
    fixmap_for_each((&ses->fls.root), node, fmn){
        file = container_of(fmn, rfile_t, snode);
        fixmap_remove(&ses->server->repo.files, file->fid, NULL);
        fclose(file->handle);
        dbstor_file_free(file->file);

    }
    pthread_mutex_unlock(&ses->server->repo.mlock);
    fixmap_fini(&ses->fls);

    fixmap_for_each((&ses->lus.root), node, fmn){
        lu = container_of(fmn, dbstor_lookup_t, node);
        dbstor_ludestroy(&ses->server->repo.dbstor, lu);
    }
    fixmap_fini(&ses->lus);

    osapi_free(ses);
}

static void session_error(struct bufferevent *bev, short error, void *ctx){
    session_t *ses = (session_t *)ctx;
    evutil_socket_t sock = bufferevent_getfd(bev);

    assert((bev != NULL) && (ctx != NULL));

    if (error & BEV_EVENT_EOF){
        log_warn("connection closed\n");
    }else if (error & BEV_EVENT_ERROR){
        log_warn("some other error\n");
    }else if (error & BEV_EVENT_TIMEOUT){
        log_warn("Timed out\n");
    }

    log_info("close session : %d\n", sock);
    session_destroy(ses);
}

static session_t *session_create(worker_t *worker, int sock){
    session_t   *ses;

    assert(worker != NULL);

    ses = osapi_malloc(sizeof(*ses));
    if(ses == NULL){
        log_warn("session create without memory!\n");
        return NULL;
    }
    memset(ses, 0, sizeof(*ses));

    ses->worker = worker;
    ses->server = worker->server;

    fixmap_node_init(&ses->node);

    fixmap_init(&ses->fls);
    fixmap_init(&ses->lus);

    evutil_make_socket_nonblocking(sock);
    ses->bev = bufferevent_socket_new(worker->evbase, sock, BEV_OPT_CLOSE_ON_FREE);
    if(ses->bev == NULL){
        log_warn("create buffer event for session fail!\n");
        osapi_free(ses);
        return NULL;
    }

    bufferevent_setcb(ses->bev, session_readcb, session_writecb,
                      session_error, (void*)ses);
    bufferevent_enable(ses->bev, EV_READ|EV_WRITE);

    return ses;
}

static void worker_readcmd(struct bufferevent *bev, void *ctx){
    worker_t   *worker = ctx;
    worker_cmd_t     cmd;
    session_t       *ses;

    assert((bev != NULL) && (ctx != NULL));

    bufferevent_read(bev, &cmd, sizeof(cmd));

    switch(cmd.cmd){
    case SERVER_WORKER_CMD_ADDSESSION:
        ses = session_create(worker, cmd.sock);

        pthread_mutex_lock(&worker->server->lock);
        ses->sesid = worker->server->sesid++;
        fixmap_insert(&worker->server->seses, ses->sesid, &ses->node);;
        pthread_mutex_unlock(&worker->server->lock);
        break;

    case SERVER_WORKER_CMD_LISTSESSION:
        break;

    case SERVER_WORKER_CMD_RMVSESSION:
        break;

    case SERVER_WORKER_CMD_TERMINATE:
        break;

    default:
        break;
    }
}

static void worker_writecmd(struct bufferevent *bev, void *ctx){
}

static void worker_errorcb(struct bufferevent *bev, short error, void *ctx){

    if (error & BEV_EVENT_EOF){
        log_warn("connection closed\n");
    }else if (error & BEV_EVENT_ERROR){
        log_warn("some other error\n");
    }else if (error & BEV_EVENT_TIMEOUT){
       log_warn("Timed out\n");
    }

    // FIXME : communication with server sock fail,
    // this worker can't accept new session
}

static void *worker_routine(void *data){
    worker_t  *worker = (worker_t *)data;

    worker->evbase = event_base_new();
    if(worker->evbase == NULL){
        log_warn("create event base in worker %d fail\n", worker->idx);
        return NULL;
    }

    evutil_make_socket_nonblocking(worker->sock_worker);
    worker->besock = bufferevent_socket_new(worker->evbase, worker->sock_worker,
                                            BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(worker->besock, worker_readcmd, worker_writecmd,
                      worker_errorcb, (void*)worker);
    bufferevent_enable(worker->besock, EV_READ|EV_WRITE);

    log_info("worker %d is running...\n", worker->idx);
    
    event_base_dispatch(worker->evbase);

    return NULL;
}

static void worker_cbs(evutil_socket_t worker, short event, void *arg){
}

static void server_accept(evutil_socket_t listener, short event, void *arg){
    struct sockaddr_storage ss;
    socklen_t    slen = sizeof(ss);
    server_t     *server = (server_t *)arg;
    worker_t     *worker;
    int          fd;
    worker_cmd_t cmd;

    fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0){
        log_warn("accept socket fail!\n");
        //FIXME: close server
    }else if (fd > FD_SETSIZE){
        close(fd);
    }else{
        cmd.cmd = SERVER_WORKER_CMD_ADDSESSION;
        cmd.sock = fd;

        if(server->worker_num == 0){
            log_warn("server without wokers!\n");
            close(fd);
        }

        server->worker_pos++;
        if(server->worker_pos >= server->worker_num)
            server->worker_pos = 0;

        worker = &(server->workers[server->worker_pos]);
        send(worker->sock_server, &cmd, sizeof(cmd), 0);

        log_info("new session is connected!\n");
    }
}

int server_listen_on(server_t *server, int sock){
    struct event	*listener;

    evutil_make_socket_nonblocking(sock);
    evutil_make_listen_socket_reuseable(sock);

    if (listen(sock, 4)<0){
        log_warn("server listen on %d fail!\n", sock);
        return -EINVAL;
    }

    listener = event_new(server->evbase, sock, EV_READ|EV_PERSIST,
        server_accept, (void*)server);
    
    event_add(listener, NULL);

    return 0;
}

int server_start(server_t *server){
    return event_base_dispatch(server->evbase);
}

int server_init(server_t *server, int workers){
    worker_t        *worker;
    evutil_socket_t socks[2];
    int         i;
    int         rc;

    assert(server != NULL);

    server->workers = osapi_malloc(workers * sizeof(*worker));
    if(server->workers == NULL){
        log_warn("server init no more memory\n");
        return -ENOMEM;
    }
    memset(server->workers, 0, workers*sizeof(*worker));

    server->evbase = event_base_new();
    if(server->evbase == NULL){
        log_warn("server init new event base instance fail\n");
        osapi_free(server->workers);
        return -EINVAL;
    }

    fixmap_init(&server->seses);
    fixmap_init(&server->users);
    pthread_mutex_init(&server->lock, NULL);

    for(i = 0; i < workers; i++){
        worker = &(server->workers[i]);

        rc = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, socks);
        if(rc != 0){
            log_warn("server init : socketpair fail:%d-%d\n", rc, errno);
            break;
        }
        worker->sock_server = socks[0];
        worker->sock_worker = socks[1];
        
        worker->idx = i;
        worker->server = server;

        if(pthread_create(&worker->wid, NULL, worker_routine, worker) != 0){
            close(socks[0]);
            close(socks[1]);
            break;
        }

        evutil_make_socket_nonblocking(worker->sock_server);
        worker->evserver = event_new(server->evbase, worker->sock_server,
                               EV_READ|EV_PERSIST, worker_cbs, (void*)server);

        event_add(worker->evserver, NULL);
    }

    server->worker_num = i;
    server->worker_pos = 0;

    return 0;
}


