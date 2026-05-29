#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/sh_server.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    char *id_str = getenv("MY_ID");
    char *shm_path = getenv("SHM_PATH");
    if (!id_str || !shm_path) return 1;
    int my_id = atoi(id_str);
    char *new_name = argv[1];

    int fd = shm_open(shm_path, O_RDWR, 0666);
    shared_ctx_t *shm = mmap(NULL, sizeof(shared_ctx_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (shm->clients[i].is_active && strcmp(shm->clients[i].name, new_name) == 0) {   
            printf("User %s already exists !\n", new_name);
            return 1;
        }
    }
    strcpy(shm->clients[my_id].name, new_name);
    printf("name change accept!\n");
 
    char broadcast_msg[1024];
    sprintf(broadcast_msg, "*** User from %s is named '%s'. ***\n", 
            shm->clients[my_id].ip_port, new_name);
            
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (shm->clients[i].is_active) {
            int fifo_fd = open(shm->clients[i].fifo_path, O_WRONLY | O_NONBLOCK);
            if (fifo_fd != -1) {
                write(fifo_fd, broadcast_msg, strlen(broadcast_msg));
                close(fifo_fd);
            }
        }
    }

    return 0;
}
