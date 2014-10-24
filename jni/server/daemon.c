
#include "server.h"
#include "logger.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <unistd.h>

static const char *rdbs = "/stor.db";
static const char *rdbu = "/user.db";

static int get_nprocs(){
	return 1;
}

int server_main(server_t *svr, struct sockaddr *addr, int len, const char *rdir){
    int sock;
    int cores;
    int rc;
 
    log_info("start server ...\n");

    cores = get_nprocs();

    rc = server_init(svr, cores*2);
    if(rc != 0){
        log_warn("server init fail:%d\n", rc);
        return rc;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        log_warn("create listenning sock fail!\n");
        return -1;
    }

    if(bind(sock, addr, len) != 0){
        log_warn("bind listenning sock fail!\n");
        return -1;
    }

    rc = dbrepo_init(&svr->repo, rdir, rdbs, rdbu);
    if(rc != 0){
        log_warn("repo init fail:%s %s %s - %d\n", rdir, rdbs, rdbu, rc);
        close(sock);
        return -1;
    }

    rc = server_listen_on(svr, sock);
    if(rc != 0){
        log_warn("server listen on port fail:%d\n", rc);
        dbrepo_fini(&svr->repo);
        close(sock);
        return -1;
    }

    log_info("server is looping ...\n");

    server_start(svr);
    
    return 0;
}

