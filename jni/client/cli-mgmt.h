
#ifndef __CLI_MGMT_H__
#define __CLI_MGMT_H__

#include "fixarray.h"
#include "cli-trans.h"

#include <unistd.h>

#define CLIENT_USER_NORMAL  0x00000001
#define CLIENT_USER_PUBLIC  0x00000002
#define CLIENT_USER_TEMP    0x00000004

int client_setup(client_t *cli, char *user, char *where, char *cred, int cred_len);
int client_close(client_t *cli);

int client_user_add(client_t *cli, char *user, char *cred, int type);
int client_user_del(client_t *cli, char *user);
int client_user_list(client_t *cli, int type, fixarray_t **users);

struct _im_system_status_rsp;
typedef struct _im_system_status_rsp cli_system_status_t;

int client_system_status(client_t *cli, cli_system_status_t **sta);
int client_network_config();

#endif

