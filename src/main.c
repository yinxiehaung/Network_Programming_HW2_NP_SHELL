#include "../include/yxsh_core.h"
#include "../include/sh_server.h"
#include <signal.h>
#define MAX_COMMAND_SIZE (1024 * sizeof(char))

int main(void) {
  char errbuf[1024];
  mem_arena_t server_arena = INIT_ARENA;
  ssize_t state = shared_arena_init(&server_arena, MiB(1), errbuf);
  if (state < 0) {
    perror(errbuf);
    return 1;
  }
   
  printf("Create Server...\n");
  net_server_t *server = net_server_init(7000, 5, &server_arena);
  shared_ctx_t *shared_state = arena_push_type(server_arena, shared_ctx_t, true, errbuf); 

  if (server == NULL || shared_state == NULL) {
    perror("Create Server Error.\n");
    return 1;
  }
  signal(SIGCHLD, SIG_IGN);
  printf("Server Listening...\n");
  while (1) {
    struct sockaddr_in client_addr;
    int client_fd = net_server_accept(server, &client_addr);
    if (client_fd < 0) continue;
    pid_t pid = fork();
    if (pid < 0) {
      perror("Server Fork Error\n");
      return 1;
    } else if (pid == 0) {
      printf("new Connect.\n");
      net_server_close(server);
      mem_arena_t session_arena = INIT_ARENA;
      state = arena_init(&session_arena, MiB(1), errbuf);
      if (state < 0) {
        perror("Session Create error.\n");
        return 1;
      }
      net_session_t *new_session = net_session_init(client_fd, 
              &client_addr, &session_arena);
      client_shell_loop(new_session, shared_state);     
    }
    close(client_fd);
  }
  net_server_close(server);
  arena_free(&server_arena);
  return 0;
}
