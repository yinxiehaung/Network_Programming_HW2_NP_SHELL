#ifndef MY_NET_INTERNAL
#define MY_NET_INTERNAL
#include "../include/my_net.h"
struct net_session_s {
    struct sockaddr_in addr;
    int sock_fd; 
    string_t buffer;
    mem_arena_t *arena;
    ssize_t last_bytes;
    int ipc_fd;
};

struct net_server_s {
    int server_fd; 
    uint16_t port;
};
#endif
