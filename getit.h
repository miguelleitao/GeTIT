/*
 * getit.h
 *
 */


#include <time.h>
#include "linmath/linmath.h"

#define SHM_NAME "/getit_state_shm"
#define SEM_NAME "/getit_sem"

//typedef float getit_vec3[3];
//typedef float getit_vec4[4];

typedef struct {
    time_t timestamp;
    vec3 position;
    vec3 orientation;
    vec3 velocity;
    vec3 angular_velocity;
    float temperature;
    float charge;
    vec4 motor_speed;
    unsigned short int flags;
    short int mode;
} getit_state;

typedef struct {
    vec3 goal_position;
    vec3 gaol_orientation;
    short int mode;
    unsigned short int flags;
} getit_cmd;

typedef struct {
    unsigned long int seq;     // incrementa a cada update
    getit_state state;
} shared_state_t;

#define Vec3Elements(v) v[0],v[1],v[2]
#define Vec4Elements(v) v[0],v[1],v[2],v[4]

#define getit_printState(state) { \
	printf("Timestamp: %ld\n", state.timestamp); \
	printf("Position: %f,%f,%f\n", Vec3Elements(state.position)); \
	printf("Orientation: %f,%f,%f\n", Vec3Elements(state.orientation)); \
	printf("Velocity: %f,%f,%f\n", Vec3Elements(state.velocity)); \
	printf("Angular Velocity: %f,%f,%f\n", Vec3Elements(state.angular_velocity)); \
	printf("Charge: %f\n", state.charge); \
	printf("\n"); \
} 
