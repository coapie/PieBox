
#include "server.h"
#include "logger.h"
#include "sharepie.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

static void usage(){
    printf("usage : iarch rootdir [service-port]\n");
    printf("    rootdir - for iarch server data\n");
    printf("    service-port - for iarch server listenning port\n");
    printf("        default port is 8086\n");
}

static sharepie_t __sp;
static struct stat rs;

int main(int argc, char *argv[]){
    char *rdir;
    short port = SERVER_DEFAULT_PORT;
    int rc;

#if 1
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
#else
    rdir = "/opt/PieBox";
#endif
    logger_init();

    rc = spie_init(&__sp);
    if(rc != 0){
        log_warn("init sharepie struct fail!\n");
        return -1;
    }

    rc = spie_set_param(&__sp, rdir, port);
    if(rc != 0){
        log_warn("set sharepie param fail:%d\n", rc);
        return -1;
    }

    spie_start(&__sp, 0);

    return 0;
}


