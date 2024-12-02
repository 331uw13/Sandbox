#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "../src/sandbox.h"
#include "../src/particles.h"

static int seed;

static struct psys_t loop_psys;
static struct psys_t rain_psys;


void loop(struct sandbox_t* sbox, void* ptr) {

    update_psys(sbox, &rain_psys);
    update_psys(sbox, &loop_psys);


    fillcircle(sbox, sbox->mouse_x, sbox->mouse_y, 6.0, 1.0, 0.3,0.3);
}

#define RAD 50
#define CX(t) (RAD*cos(t*3))
#define CY(t) (RAD*sin(t*3))


// ------ colorful worm loop particle system

void loop_psys_pupdate(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {

    p->x += p->vel_x;
    p->y += p->vel_y;

    p->vel_x += p->acc_x;
    p->vel_y += p->acc_y;

    p->r = lerp(p->lifetime, p->init_red, 0.0);
    p->g = lerp(p->lifetime, p->init_grn, 0.0);
    p->b = lerp(p->lifetime, p->init_blu, 0.0);


    fillcircle(sbox, p->x, p->y, lerp(p->lifetime, 4.0, 0.0), p->r, p->g, p->b);
}


void loop_psys_pdeath(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {
    
    p->x = sbox->center_col + CX(sbox->time + 2.0*cos(sbox->time));
    p->y = sbox->center_row + CY(sbox->time);


    p->vel_x = randomf(&seed, -0.1, 0.1);
    p->vel_y = randomf(&seed, -0.1, 0.1);


    p->acc_x = 0.0;
    p->acc_y = 0.008;

    rainbow_palette(sin(sbox->time), &p->r, &p->g, &p->b);

    p->init_red = p->r;
    p->init_grn = p->g;
    p->init_blu = p->b;

    p->alive = 1;
    p->max_lifetime = randomf(&seed, 0.2, 1.0);
    p->lifetime = 0.0;

}

// ------- rain particle system

void rain_particle_respawn(struct sandbox_t* sbox, struct particle_t* p) {
    p->x = randomi(&seed, 0, sbox->max_col);
    p->y = 0;

    p->vel_y = randomf(&seed, 2.0, 4.0);
    p->vel_x = randomf(&seed, -0.2, -0.5);

    p->r = 0.05;
    p->g = 0.05;
    p->b = randomf(&seed, 0.4, 1.0);

    p->alive = 1;
    p->max_lifetime = 10.0;
    p->lifetime = 0.0;
}

void rain_psys_pupdate(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {
    p->y += p->vel_y;
    p->x += p->vel_x;
    if(p->y > sbox->max_row) {
        rain_particle_respawn(sbox, p);
    }

    setpixel(sbox, p->x, p->y, p->r, p->g, p->b);
}


void rain_psys_pdeath(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {
    rain_particle_respawn(sbox, p);
}


void setup(struct sandbox_t* sbox) {

    init_psys(sbox, &loop_psys, 80,
            PSYSNOSETUP, loop_psys_pupdate, loop_psys_pdeath, NULL);

    init_psys(sbox, &rain_psys, 10,
            PSYSNOSETUP, rain_psys_pupdate, rain_psys_pdeath, NULL);

}



int main() {

    struct sandbox_t sbox;
    if(!init_sandbox(&sbox, 700, 600, "[Sandbox]")) {
        return 1;
    }

    seed = time(0);

    setup(&sbox);
    run_sandbox(&sbox, loop, NULL);

    delete_psys(&loop_psys);
    delete_psys(&rain_psys);
    free_sandbox(&sbox);

    return 0;
}



