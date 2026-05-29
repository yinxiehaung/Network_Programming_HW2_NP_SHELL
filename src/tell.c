#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/sh_server.h"

int main(int argc, char *argv[]) {
    if (argc < 3) return 1; 

    char *id_str = getenv("MY_ID");
    char *shm_path = getenv("SHM_PATH");
    if (!id_str || !shm_path) return 1;
    int my_id = atoi(id_str);
    int target_id = atoi(argv[1]);

    int fd = shm_open(shm_path, O_RDWR, 0666);
    shared_ctx_t *shm = mmap(NULL, sizeof(shared_ctx_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (target_id <= 0 || target_id > MAX_CLIENTS || !shm->clients[target_id].is_active) {
        printf("*** Error: user #%d does not exist yet. ***\n", target_id);
        return 1;
    }

 
    char user_msg[1024] = {0};
    for (int i = 2; i < argc; i++) {
        strcat(user_msg, argv[i]);
        if (i < argc - 1) strcat(user_msg, " ");
    }

    char final_msg[2048];
    sprintf(final_msg, "<%s(%d) told you>: %s\n", shm->clients[my_id].name, my_id, user_msg);

    int fifo_fd = open(shm->clients[target_id].fifo_path, O_WRONLY | O_NONBLOCK);
    if (fifo_fd != -1) {
        write(fifo_fd, final_msg, strlen(final_msg));
        close(fifo_fd); 
        printf("send accept!\n");
    }
    return 0;
}
