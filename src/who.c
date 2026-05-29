#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/sh_server.h"

int main(int argc, char *argv[]) {
    char *id_str = getenv("MY_ID");
    char *shm_path = getenv("SHM_PATH");
    if (!id_str || !shm_path) { 
        return 1;
    }
    int my_id = atoi(id_str);
    int fd = shm_open(shm_path, O_RDWR, 0666);
    if (fd < 0) {
        perror("open error:");
        return 1;
    }
    shared_ctx_t *shm = mmap(NULL, sizeof(shared_ctx_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  
    fflush(stdout);
    printf("<ID>\t<nickname>\t<IP:port>\t<indicate me>\n");
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (shm->clients[i].is_active) {
            printf("%d\t%s\t%s\t%s\n", 
                   shm->clients[i].id,
                   shm->clients[i].name,
                   shm->clients[i].ip_port,
                   (i == my_id) ? "<-me" : "");
        }
    }
    
    return 0;
}
