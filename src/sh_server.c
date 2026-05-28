#include "../include/yxsh_core.h"
#include "../include/sh_server.h"
#include "my_net_internal.h"


static int get_my_id(shared_ctx_t *shared_state) {
  int my_id = -1;
  for (int i = 1; i <= MAX_CLIENTS; i++) {
    if (!shared_state->clients[i].is_active) {
      my_id = i;
      shared_state->clients[i].is_active = 1;
      shared_state->clients[i].id = my_id;
      shared_state->clients[i].pid = getpid();
      strcpy(shared_state->clients[i].name, "no name");
      return my_id;
    }
  }
  perror("Connect FULL.");
  exit(1);
}

void client_shell_loop(net_session_t *session, shared_ctx_t *shared_state) {
  dup2(session->sock_fd,STDERR_FILENO);
  dup2(session->sock_fd,STDOUT_FILENO);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

 
  int my_id = get_my_id(shared_state);
  char *ip = inet_ntoa(session->addr.sin_addr);
  int port = ntohs(session->addr.sin_port);
  sprintf(shared_state->clients[my_id].ip_port, "%s/%d", ip, port);
  
  sprintf(shared_state->clients[my_id].fifo_path, "/tmp/yxsh_%d_fifo", my_id);
  mkfifo(shared_state->clients[my_id].fifo_path, 0666);
  int my_fifo_fd = open(shared_state->clients[my_id].fifo_path, O_RDWR | O_NONBLOCK);

  net_session_add_watch_fd(session, my_fifo_fd);

  shell_ctx_t gctx;
  char errbuf[1024];
  init_shell_ctx(&gctx, session->arena, shared_state, my_id, errbuf); 

  while (1) {
    printf("%% ");
    fflush(stdout);
    string_t event_data = INIT_STRING;
    session_event_t event = net_session_wait_event(session, &event_data);
    switch(event){
    case EVENT_DISCONNECT: 
      shared_state->clients[my_id].is_active = false;      
      unlink(shared_state->clients[my_id].fifo_path);
      net_session_close(session);
      exit(0);
    case EVENT_COMMAND:
      gctx.command = event_data;
      yxsh_run(&gctx);
      break;
    case EVENT_IPC:
      printf("%.*s\n",(int)event_data.len, event_data.str);
      break;
    case EVENT_NONE:
      break;
    }
  } 
}
