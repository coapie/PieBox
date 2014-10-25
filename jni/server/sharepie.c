
#include "sharepie.h"
#include "osapi.h"
#include "logger.h"

#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sysinfo.h>

static const char *rdbs = "/stor.db";
static const char *rdbu = "/user.db";

#ifdef ANDROID_NDK
static int get_nprocs(){
	return 1;
}
#endif

int spie_init(sharepie_t *sp){
    memset(sp, 0, sizeof(*sp));
    
    sp->port = 8086;
    
    return 0;
}

int spie_fini(sharepie_t *sp){
    
    if(sp->rdir != NULL){
        osapi_free(sp->rdir);
        sp->rdir = NULL;
    }

    return 0;
}

int spie_set_param(sharepie_t *sp, const char *rdir, short port){
    int len;

    log_info("spie set param : %s %d\n", rdir, port);

    sp->port = port;

    len = strlen(rdir) + 1;
    sp->rdir = osapi_malloc(len);
    if(sp->rdir == NULL){
    	log_warn("malloc memory for spie rdir fail\n");
        return -ENOMEM;
    }
    strcpy(sp->rdir, rdir);

    return 0;
}

static int spie_daemon(sharepie_t *sp, int nochdir, int  noclose)
{
   pid_t  pid;

   if(!nochdir && chdir("/") != 0){
       log_warn("change dir fail\n");
       return -1;
   }

   if(!noclose)
   {
     int  fd = open("/dev/null", O_RDWR);

     if(fd < 0){
         log_warn("open /dev/null for redirect stdin fail\n");
         return -1;
     }

     if(dup2(fd, 0) < 0 || dup2( fd, 1 ) < 0 || dup2( fd, 2 ) < 0) 
     {
         log_warn("dup stdin stdout stderr fail\n");
         close(fd);
         return -1;
     }

     close(fd);
  }
  
   pid = fork();
   if(pid < 0){
       log_warn("for daemon process fail\n");
       return -1;
   }

   if (pid > 0){
       sp->pid = pid;
       return 0;
   }

   log_info("run in daemon process...\n");

   if (setsid() < 0){
       log_warn("setsid in daemon fail!\n");
       return -1;
   }

   return 1;
}

void spie_signal_usr1_handler(int sig){
    log_info("receive usr1 signal, exit PieBox\n");
    _exit(0);
}

int spie_start(sharepie_t *sp, int bg){
    struct sockaddr_in  addr;
    struct sigaction action;
    int sock;
    int rc;
 
    if(bg){
        rc = spie_daemon(sp, 1, 1);
        switch(rc){
        case 0:
            log_info("share pie is run in background\n");
            return 0;

        case 1:
            log_info("share pie daemon is running...\n");
 //           logger_fini();
 //           logger_init();
            break;

        default:
            log_warn("start share pie fail:%d\n", rc);
            return rc;
        }
    }
    // set sigaction function
    action.sa_handler = spie_signal_usr1_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGUSR1, &action, NULL)){
        log_warn("set signal action handler fail\n");
        return -1;
    }


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        log_warn("create listenning sock fail!\n");
        return -1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = htons(sp->port);
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0){
        log_warn("bind listenning sock fail!\n");
        return -1;
    }
    
    rc = server_init(&sp->server, 2*get_nprocs());
    if(rc != 0){
        log_warn("server init fail:%d\n", rc);
        return -1;
    }

    rc = dbrepo_init(&sp->server.repo, sp->rdir, rdbs, rdbu);
    if(rc != 0){
        log_warn("repo init fail:%s %s %s - %d\n", sp->rdir, rdbs, rdbu, rc);
        close(sock);
        return -1;
    }

    rc = server_listen_on(&sp->server, sock);
    if(rc != 0){
        log_warn("server listen on port fail:%d\n", rc);
        dbrepo_fini(&sp->server.repo);
        close(sock);
        return -1;
    }

    log_info("server is looping ...\n");

    server_start(&sp->server);

    return 0;
}

int spie_stop(sharepie_t *sp){
    int rc;

    rc = kill(sp->pid, SIGUSR1);
    if(rc == 0){
        // sleep 10ms, let worker exit!
        usleep(10000);
        return 0;
    }

    return rc;
}

int spie_restart(sharepie_t *sp, int bg){
    int rc;

    rc = spie_stop(sp);
    if(rc != 0)
        return rc;

    return spie_start(sp, bg);
}

int spie_clean(sharepie_t *sp){
    int rc;

    rc = spie_stop(sp);
    if(rc != 0)
        return rc;

    rc = dbrepo_clean(&sp->server.repo);

    return rc;
}

