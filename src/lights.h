#ifndef SANDBOX_LIGHTS_H
#define SANDBOX_LIGHTS_H


// RENAME
#define MAX_LIGHTS 16


struct light_t {
    
    float x; 
    float y;

    float r;
    float g;
    float b;

    float strength;
    float radius;
    float effect;

    unsigned int index;
};

struct sbp_t;


// 'setup_light' doesnt allocate memory
// but returns a pointer to elemnt that was setup at 'sbox->lights'
// you can request a index position with 'req_index'
// NULL is returned if 'req_index' is bigger or equal to MAX_LIGHTS
// if 'req_index' is less than zero it is same as passing 'sbox->num_lights'
// note if sbox->num_lights has reached the max user has to specify 'req_index'
struct light_t* sb_init_light(
        struct sbp_t* sbox,
        int req_index,
        float x, float y,
        float r,
        float g,
        float b,
        float strength,
        float radius,
        float effect
        );

// update everything for the light
void sb_update_light_allvars(struct sbp_t* sbox, struct light_t* light);

void sb_update_light_pos(struct sbp_t* sbox, struct light_t* light);
void sb_update_light_color(struct light_t* light);
void sb_update_light_strength(struct light_t* light);
void sb_update_light_radius(struct light_t* light);
void sb_update_light_effect(struct light_t* light);


#endif
