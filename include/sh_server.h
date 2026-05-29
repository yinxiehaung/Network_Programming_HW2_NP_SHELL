#ifndef SH_SERVER
#define SH_SERVER
#include "my_net.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX_CLIENTS 30
typedef struct client_ctx_s client_ctx_t;
typedef struct shared_ctx_s shared_ctx_t;

struct client_ctx_s{
  bool is_active;
  int id;
  pid_t pid;
  char name[32];
  char fifo_path[64];
  char ip_port[32];
};

struct shared_ctx_s {
  client_ctx_t clients[MAX_CLIENTS + 1];
};

void client_shell_loop(net_session_t *, shared_ctx_t *);
#endif
