#include "../include/yxsh_core.h"
#include "../include/sh_server.h"
#include <signal.h>
#define MAX_COMMAND_SIZE (1024 * sizeof(char))

void handle_sigint(int sig) {
    shm_unlink("/yxsh_mem"); 
    exit(0);
}


int main(void) {
  signal(SIGINT, handle_sigint);
  char errbuf[1024];
  mem_arena_t server_arena = INIT_ARENA;
  ssize_t state = arena_init(&server_arena, MiB(1), errbuf);
  if (state < 0) {
    perror(errbuf);
    return 1;
  }
   
  printf("Create Server...\n");
  net_server_t *server = net_server_init(7000, 5, &server_arena);
 
  int shm_fd = shm_open("/yxsh_mem", O_CREAT | O_RDWR | O_TRUNC, 0666);
  if (shm_fd < 0) {
    perror("The shm_open Fail.\n");
    return 1;
  }
  ftruncate(shm_fd, sizeof(shared_ctx_t));
  shared_ctx_t *shared_state = mmap(NULL, sizeof(shared_ctx_t), 
          PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 

  if (server == NULL || shared_state == NULL) {
    perror("Create Server Error.\n");
    return 1;
  } 
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
