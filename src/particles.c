#include <stdio.h>
#include <stdlib.h>

#include "sandbox.h"
#include "particles.h"



int sb_init_psys(
        struct sbp_t* sbox,
        struct psys_t* psys,
        size_t num_particles,
        
        void(*setup_callback)
        (struct sbp_t*, struct psys_t*, struct particle_t*),

        void(*update_callback)
        (struct sbp_t*, struct psys_t*, struct particle_t*),
        
        void(*death_callback)
        (struct sbp_t*, struct psys_t*, struct particle_t*),
        
        void* userptr)
{
    int res = 0;

    if(!psys) {
        goto error;
    }

    if(!update_callback) {
        fprintf(stderr, "[ERROR] Particle system requires 'update_callback'\n");
        goto error;
    }


    psys->particles = NULL;
    psys->userptr = userptr;
    psys->ready = 0;
    
    psys->num_particles = num_particles;
    psys->particles = malloc(sizeof *psys->particles * num_particles);
    psys->update_callback = update_callback;
    psys->death_callback = death_callback;

    psys->last_dead_index = 0;


    if(!psys->particles) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for particle system.\n");
        perror("malloc");
    }

    void(*pinit_callback)(struct sbp_t*, struct psys_t*, struct particle_t*)
        = ((setup_callback != NULL) ? setup_callback 
        : ((death_callback != NULL) ? death_callback : NULL));

    if(!pinit_callback) {
        fprintf(stderr, 
                "[ERROR] %s | no setup_callback or death_callback.\n"
                "    particles can not be initialized.\n", __func__);
        goto error;
    }

    for(size_t i = 0; i < num_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p) {
            goto error;
        }
        
        p->x = 0.0;
        p->y = 0.0;
        p->r = 0.0;
        p->g = 0.0;
        p->b = 0.0;
        p->vel_x = 0.0;
        p->vel_y = 0.0;
        p->acc_x = 0.0;
        p->acc_y = 0.0;
        p->alive = 0;
        p->lifetime = 0.0;
        p->max_lifetime = 0.0;
        p->userptr = NULL;
        p->index = i;

        pinit_callback(sbox, psys, p);
    }

    psys->alldead = 0;
    psys->ready = 1;
    res = 1;


error:
    return res;
}

void sb_delete_psys(struct psys_t* psys) {
    if(!psys) {
        return;
    }

    if(psys->particles) {
        free(psys->particles);
        psys->particles = NULL;
    }

    psys->ready = 0;
    psys->num_particles = 0;
    psys->last_dead_index = 0;
}

void sb_update_psys(struct sbp_t* sbox, struct psys_t* psys) {
    if(!psys) {
        return;
    }
    if(psys->alldead) {
        return;
    }

    if(!psys->update_callback) {
        return;
    }

    int alldead = 1;

    for(size_t i = 0; i < psys->num_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p) {
            continue;
        }

        if(p->alive) {
            alldead = 0;
            p->lifetime += sbox->dt;
            if(p->lifetime >= p->max_lifetime) {
                p->alive = 0;
                if(psys->death_callback) {
                    psys->last_dead_index = i;
                    psys->death_callback(sbox, psys, p);
                }
            }
       
            psys->update_callback(sbox, psys, p);
        }
    }

    psys->alldead = alldead;
}

void sb_revive_psys(struct psys_t* psys) {
    if(psys->ready) {
        for(size_t i = 0; i < psys->num_particles; i++) {
            psys->particles[i].alive = 1;
        }
        psys->alldead = 0;
    }
}

