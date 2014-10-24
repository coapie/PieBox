
#ifndef __SHAREPIE_H__
#define __SHAREPIE_H__

#include "server.h"

#include <unistd.h>

typedef struct _sharepie{
    server_t server;
    pid_t pid;
    short port;
    char *rdir;
}sharepie_t;

int spie_init(sharepie_t *sp);
int spie_fini(sharepie_t *sp);

int spie_set_param(sharepie_t *sp, const char *rdir, short port);
int spie_start(sharepie_t *sp, int bg);
int spie_stop(sharepie_t *sp);
int spie_restart(sharepie_t *sp, int bg);
int spie_clean(sharepie_t *sp);

#endif // __SHAREPIE_H__

