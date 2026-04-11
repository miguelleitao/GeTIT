#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>
#include "getit.h"

time_t initTime = 0L;

void getit_getState(getit_state *state) {
    time_t now = time(NULL);

    double tNow   = (double)now-(double)initTime;
    double tDelta = (double)now-(double)state->timestamp;
    state->timestamp = now;
    for( int i=0 ; i<3 ; i++ ) {
        state->velocity[i]         = 0.5 * sin(tNow/10./(float)(i+1));
        state->angular_velocity[i] = 0.1 * sin(tNow/ 5./(float)(i+1));
	}
	state->velocity[1] *= 10.;
    state->orientation[0] *= 0.1;
    state->orientation[1] *= 0.2;
	
    
    quat q;
    quat_make_from_euler_vec(q, state->orientation);
 
    quat wq;
    quat_set_vec(wq, 0., state->angular_velocity);

    quat dq;
    quat_muled(dq, q, wq);   	// q ⊗ ω
    quat_mul_scalar(dq, 0.5f * tDelta);

    quat_add(q, dq);
    quat_normalize(q);
    
    quat_to_euler(state->orientation, q);
    
    // --- posição ---
	vec3 v_world;
	quat_mul_vec3(v_world, q, state->velocity);

	vec3_scale(v_world, tDelta);
	vec3_add(state->position, v_world);
}

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    if (ftruncate(fd, sizeof(shared_state_t)) < 0) {
        perror("ftruncate"); return 1;
    }

    shared_state_t *shared = mmap(NULL, sizeof(shared_state_t),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, fd, 0);

    if (shared == MAP_FAILED) {
        perror("mmap"); return 1;
    }

    // inicializar memória
    memset(shared, 0, sizeof(*shared));

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open"); return 1;
    }

    getit_state gState = {
        .position = {0.0f, 0.0f, 0.0f},
        .orientation = {0.0f, 0.0f, 0.0f},
        .velocity = {0.0f, 0.0f, 0.0f},
        .angular_velocity = {0.0f, 0.0f, 0.0f},
        .temperature = 30.0f,
        .charge = 100.0f,
        .motor_speed = {0.0f, 0.0f, 0.0f},
        .flags = 0,
        .mode = 0
    };

    unsigned long int seq = 0;
    
    initTime = time(NULL);
    printf("InitTime: %ld\n", initTime);

    while (1) {
        // State update
        getit_getState(&gState);

        // escrever na shm
        shared->state = gState;
        shared->seq = ++seq;

        // sinalizar consumidor
        sem_post(sem);
        getit_printState(gState);

        sleep(1);
    }

    return 0;
}

