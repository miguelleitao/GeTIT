// sender.c

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "getit.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

shared_state_t *shared;

#define SOCKET_PATH "/tmp/getit_l1.sock"

time_t initTime = 0L;
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
/*
void getit_initState(getit_state *state) {
    time_t now = time(NULL);
	state->timestamp = now;
	state->position = {0.0f, 0.0f, 0.0f};
	state->orientation = {0.0f, 0.0f, 0.0f};
	state->velocity = {0.0f, 0.0f, 0.0f};
	state->angular_velocity = {0.0f, 0.0f, 0.0f};
	state->temperature = 30.0f;
	state->charge = 100.0f;        // exemplo de valor inicial
	state->motor_speed = {0.0f, 0.0f, 0.0f, 0.0f},
	state->flags = 0;
	state->mode = 0;
}
*/
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

int getitsend(void) {
    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    const char *msg = "hello from sender";

    if (sendto(fd, msg, strlen(msg), 0,
            (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        return 1;
    }
    printf("Mensagem enviada\n");

    close(fd);
    return 0;
}

int shm_init() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return -1; }

    if (ftruncate(fd, sizeof(shared_state_t)) < 0) {
        perror("ftruncate"); return -1;
    }

    shared = mmap(NULL, sizeof(shared_state_t),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap"); return -1;
    }
    return 0;
}

int main() {
	initTime = time(NULL);
	getit_state gState = {	
		.timestamp = initTime,
		.position = {0.0f, 0.0f, 0.0f},
		.orientation = {0.0f, 0.0f, 0.0f},
		.velocity = {0.0f, 0.0f, 0.0f},
		.angular_velocity = {0.0f, 0.0f, 0.0f},
		.temperature = 30.0f,
		.charge = 100.0f,        // exemplo de valor inicial
		.motor_speed = {0.0f, 0.0f, 0.0f, 0.0f},
		.flags = 0,
		.mode = 0
	};
	int End = 0;
	unsigned long int seq = 0;

	if ( shm_init() ) return 1; 
	
	while( ! End ) {
		getit_getState(&gState);
		// escrever para memória partilhada
		shared->state = gState;
		shared->seq = ++seq;
		getit_printState(gState);
		sleep(1);
	}
}
