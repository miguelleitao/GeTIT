#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "getit.h"

int main() {
    int fd;
    while ((fd = shm_open(SHM_NAME, O_RDONLY, 0666)) < 0) {
		perror("waiting for shm...");
		sleep(1);
	}
    printf("got shm\n");
    shared_state_t *shared = mmap(NULL, sizeof(shared_state_t),
                                  PROT_READ,
                                  MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap"); return 1;
    }

    printf("got mmap\n");
	sem_t *sem;
	while ((sem = sem_open(SEM_NAME, 0)) == SEM_FAILED) {
		perror("waiting for semaphore...");
		sleep(1);
	}

    printf("got sem\n");
    while (1) {
        // bloqueia até haver novo estado
        sem_wait(sem);

        getit_state s = shared->state;

		getit_printState(s);
    }

    return 0;
}

