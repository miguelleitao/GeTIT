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
    for( int i=0 ; i<3 ; i++ ) 
        state->velocity[i] = 10. * sin(tNow/10./(float)(i+1));
    
    quat q;
    quat_make_from_euler_vec(q, state->orientation);
    mat4x4 m;
    quat_to_mat4x4(m, q);
    vec3 v;
    vec3_scaled(v, state->velocity, tDelta);
    vec4 vh;
    vec4f_from_vec3(vh, v);
    vec4 d;
    // R(euler) * vel_body * dt
    mat4x4_mul_vec4(d, m, vh);
    vec3_add(state->position, d);

    state->timestamp = now;
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

