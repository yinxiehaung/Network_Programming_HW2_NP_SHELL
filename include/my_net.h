#ifndef MY_NET_H
#define MY_NET_H
#include "my_arena.h"
#include "my_string.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

typedef uint16_t ui16;
typedef uint32_t ui32;
typedef struct net_session_s net_session_t;
typedef struct net_server_s net_server_t;

typedef enum {
    EVENT_DISCONNECT = -1,
    EVENT_NONE = 0,
    EVENT_COMMAND = 1,
    EVENT_IPC = 2
} session_event_t;

net_server_t *net_server_init(ui16, ui32, mem_arena_t *);
int  net_server_accept(net_server_t *, struct sockaddr_in *);
void net_server_close(net_server_t *);
net_session_t *net_session_init(int, struct sockaddr_in *, mem_arena_t *);
ssize_t net_session_recv(net_session_t *, string_t *);
ssize_t net_session_send(net_session_t *, const char *);
ssize_t get_session_recv_last_bytes(net_session_t *);
void net_session_close(net_session_t *);
void net_session_add_watch_fd(net_session_t *, int fd);
session_event_t net_session_wait_event(net_session_t *, string_t *);
#endif 
