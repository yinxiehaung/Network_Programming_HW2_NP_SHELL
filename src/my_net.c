#include "../include/my_net.h"
#include "my_net_internal.h"

net_server_t *net_server_init(ui16 port, ui32 backlog, mem_arena_t *arena) {
  mem_tmp_arena_t check_point = arena_begin_tmp(arena);
  net_server_t *server = arena_push_type(*arena, net_server_t, true, NULL);

  if (server == NULL || (server->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto free_server;
  }
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  int ret = 0, reuse = 1;
  ret = setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(int));
  if (ret < 0 || (bind(server->server_fd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
    goto free_server; 
  }
  listen(server->server_fd, backlog);
  return server;
free_server:
  arena_end_tmp(&check_point);
  return NULL;
}

int net_server_accept(net_server_t *server, struct sockaddr_in *client) {
  socklen_t client_len = sizeof(struct sockaddr_in); 
  int socket_client = accept(server->server_fd, (struct sockaddr *)client, &client_len);
  return socket_client;
}

void net_server_close(net_server_t *server) {
  close(server->server_fd);
  server->port = 0;
  return;
}

net_session_t *net_session_init(int fd, struct sockaddr_in *client_addr, mem_arena_t *arena) {
  net_session_t *session = arena_push_type(*arena, net_session_t, true, NULL);
  if (session == NULL) {
    return NULL;
  }
  session->addr = *client_addr;
  session->sock_fd = fd;
  session->arena = arena; 
  session->ipc_fd = -1;
  return session;
}

ssize_t net_session_recv(net_session_t *session, string_t *out)
{
    char buf[1024] = {};
    ssize_t recv_state = recv(session->sock_fd, buf, sizeof(buf) - 1, 0);
    if (recv_state <= 0) {
        return -1;
    }
    session->last_bytes = recv_state;
    if (session->buffer.str == NULL) {
        session->buffer = str_new_in(*session->arena, "", 0);
    }
    str_cat_char_to_end_in(*session->arena, session->buffer, buf);
    ssize_t newline_index = str_index_of(&session->buffer, "\n", STR_FIND_AUTO); 
    if (newline_index >= 0) {
        ssize_t actual_cmd_len = newline_index;
        if (newline_index > 0 && session->buffer.str[newline_index - 1] == '\r') {
            actual_cmd_len = newline_index - 1;
        }
        string_t tmp = str_substr_in(*session->arena, session->buffer, 0, actual_cmd_len);
        *out = tmp;
        ssize_t remaining_len = session->buffer.len - (newline_index + 1);
        memmove(session->buffer.str, 
                session->buffer.str + newline_index + 1, 
                remaining_len);
        session->buffer.len = remaining_len;
        session->buffer.str[remaining_len] = '\0';
        return 1;
    }
    return 0;
}

ssize_t get_session_last_bytes(net_session_t *session) 
{
    return session->last_bytes; 
}

ssize_t net_session_send(net_session_t *session, const char *buf)
{
    return send(session->sock_fd, buf, strlen(buf), 0);
} 

void net_session_add_watch_fd(net_session_t *session, int fd) 
{
    session->ipc_fd = fd;
}

session_event_t net_session_wait_event(net_session_t *session, string_t *out_data) 
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(session->sock_fd, &read_fds);

    int max_fd = session->sock_fd;

    if (session->ipc_fd != -1) {
        FD_SET(session->ipc_fd, &read_fds);
        if (session->ipc_fd > max_fd) {
            max_fd = session->ipc_fd;
        }
    }

    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
        return EVENT_NONE;
    }
    
    if (FD_ISSET(session->sock_fd, &read_fds)) {
        int status = net_session_recv(session,out_data);
        if (status <= 0) {
            return EVENT_DISCONNECT;
        }
        return EVENT_COMMAND;
    }

    if (session->ipc_fd != -1 && FD_ISSET(session->ipc_fd, &read_fds)) {
        char buf[1024];
        int bytes = read(session->ipc_fd, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            *out_data = str_new_in(*session->arena, buf, bytes);
            return EVENT_IPC;
        }
    }
    return EVENT_NONE;
}

void net_session_close(net_session_t *session) 
{
    close(session->sock_fd);
    arena_free(session->arena);
    return;
}

