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

    int fd = shm_open(shm_path, O_RDWR, 0666);
    shared_ctx_t *shm = mmap(NULL, sizeof(shared_ctx_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    char user_msg[1024] = {0};
    for (int i = 1; i < argc; i++) {
        strcat(user_msg, argv[i]);
        if (i < argc - 1) strcat(user_msg, " ");
    }

    char final_msg[2048];
    sprintf(final_msg, "<%s(%d) yelled>: %s\n", shm->clients[my_id].name, my_id, user_msg);

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (shm->clients[i].is_active) {
            int fifo_fd = open(shm->clients[i].fifo_path, O_WRONLY | O_NONBLOCK);
            if (fifo_fd != -1) {
                write(fifo_fd, final_msg, strlen(final_msg));
                close(fifo_fd);
            }
        }
    }
    return 0;
}
