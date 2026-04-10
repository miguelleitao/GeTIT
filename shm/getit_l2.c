#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "getit.h"

#include <sys/mman.h>
#include <fcntl.h>



shared_state_t *shared;
/*
void getit_printState(getit_state *state) {
	printf("Timestamp: %ld\n", state->timestamp);
	printf("Position: %f,%f,%f\n", Vec3Elements(state->position));
	printf("Orientation: %f,%f,%f\n", Vec3Elements(state->orientation));
	printf("Velocity: %f,%f,%f\n", Vec3Elements(state->velocity));
	printf("Angular Velocity: %f,%f,%f\n", Vec3Elements(state->angular_velocity));
	printf("Charge: %f\n", state->charge);
	printf("\n");
}
*/
int main() {
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);

    shared = mmap(NULL, sizeof(shared_state_t),
                  PROT_READ, MAP_SHARED, fd, 0);

    unsigned long int last_seq = 0;

    while (1) {
        if (shared->seq != last_seq) {
            last_seq = shared->seq;

            getit_state s = shared->state; // cópia local
			getit_printState(s);
            printf("Temp: %.1f | Charge: %.1f\n",
                   s.temperature, s.charge);
        }
        usleep(10000); // 10 ms
    }
}
