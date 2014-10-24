
#include "server.h"
#include "logger.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

extern int server_main(server_t *svr, struct sockaddr *addr, int len, 
        const char *rdir);

static void usage(){
    printf("usage : iarch rootdir [service-port]\n");
    printf("    rootdir - for iarch server data\n");
    printf("    service-port - for iarch server listenning port\n");
    printf("        default port is 8086\n");
}
    
static server_t svr;
static struct sockaddr_in sin;
static struct stat rs;

int main(int argc, char *argv[]){
    char *rdir;
    short port = SERVER_DEFAULT_PORT;
    int rc;

    if(argc < 2){
        usage();
        return -1;
    }
    rdir = argv[1];

    rc = stat(rdir, &rs);
    if(rc != 0){
        usage();
        return -1;
    }

    if(argc > 2){
        port = (short)atoi(argv[2]);
    }

    logger_init();

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    server_main(&svr, (struct sockaddr *)&sin, sizeof(sin), rdir);

    return 0;
}


