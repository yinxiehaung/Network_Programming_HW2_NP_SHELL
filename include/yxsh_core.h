#ifndef YXSH_H
#define YXSH_H
#include "my_arena.h"
#include "my_string.h"
#include "sh_server.h"

#define NUM_PIPE_MAX 128
typedef struct shell_ctx_s shell_ctx_t;

struct shell_ctx_s {
  int exit_status;
  int pipe_buffer[NUM_PIPE_MAX][2];
  string_t command;
  ui64 command_counter;
  mem_arena_t *arena;
  shared_ctx_t *shared_state;
  int my_id;
};

void init_shell_ctx(shell_ctx_t *, mem_arena_t *, shared_ctx_t *, int, char *);
int yxsh_run(shell_ctx_t *);
#endif
