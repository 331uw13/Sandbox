#ifndef PARTICLES_H
#define PARTICLES_H

#include <stddef.h>


struct sbp_t;


struct particle_t {

    float x;
    float y;

    float vel_x;
    float vel_y;

    float acc_x;
    float acc_y;

    float r;
    float g;
    float b;

    float init_red;
    float init_grn;
    float init_blu;

    int   alive;
    float lifetime;
    float max_lifetime;

    size_t index;

    void* userptr; // for user
};

// if user has set the 'death_callback' to NULL
// and when particle dies 
// it cannot be updated by 'update_callback' anymore.
// this way the user can make particle systems where
// particles are updated until all of them are dead
// particle system can be then "re-activated" by 'revive_psys()'

// 'revive_psys()' also sets 'alldead' to 0

// see ./demos/particle_demo.c for some examples.

struct psys_t {
    struct particle_t* particles;
    size_t num_particles;
    size_t last_dead_index;

    // called when particle needs updating for frame.
    void(*update_callback)(
            struct sbp_t*,
            struct psys_t*,
            struct particle_t*
            );

    // called when particle has died so user can "respawn" it.
    // can be null
    void(*death_callback)(
            struct sbp_t*,
            struct psys_t*,
            struct particle_t*
            );

    void* userptr; // for user
    int alldead; // if this is set to 1 'update_psys()' has no effect.

    int ready;
};

// RENAME
#define PSYSNOSETUP NULL
#define PSYSNORESPAWN NULL

int init_psys(
        struct sbp_t* sbox,
        struct psys_t* psys,
        size_t num_particles,

        // setup the initial positions for particles..
        // if this was set to 'PSYSNOSETUP'.
        // then 'death_callback' is required,
        //      it is called when initializing particles.
        void(*setup_callback)
        (struct sbp_t*, struct psys_t*, struct particle_t*),
        
        // called every frame ifif particle is alive
        void(*update_callback)
        (struct sbp_t*, struct psys_t*, struct particle_t*),
        
        // called when particle dies
        // can be NULL. read above.
        void(*death_callback) 
        (struct sbp_t*, struct psys_t*, struct particle_t*),
        

        void*  userptr
        );

void delete_psys(struct psys_t* psys);

void update_psys(struct sbp_t* sbox, struct psys_t* psys);
void revive_psys(struct psys_t* psys);


#endif //PARTICLES_H
