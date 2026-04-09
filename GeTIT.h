/*
 * getit.h
 *
 */

typedef float getit_vec3[3];

typedef struct {
    getit_vec3 position;
    getit_vec3 orientation;
    getit_vec3 velocity;
    getit_vec3 angular_velocity;
    float temperature;
    float charge;
    getit_vec3 motor_speed;
    unsigned short int flags;
    short int mode;
} getit_state;

typedef struct {
    getit_vec3 goal_position;
    getit_vec3 gaol_orientation;
    short int mode;
    unsigned short int flags;
}
