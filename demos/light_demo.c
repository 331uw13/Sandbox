#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "../src/sandbox.h"
#include "../src/particles.h"


static int seed;
static struct psys_t   psystem;
static struct light_t*  mylight_A;
static struct light_t*  mylight_B;
static struct light_t*  g_light_selected;


void change_light_strength(struct sandbox_t* sbox, struct light_t* light) {
    if(sbox->mouse_scroll < 0) {
        light->strength -= 0.1;
    }
    else if(sbox->mouse_scroll > 0) {
        light->strength += 0.1;
    }
    update_light_strength(light);


}

void loop(struct sandbox_t* sbox, void* ptr) {


    rainbow_palette(sin(sbox->time*0.5), &mylight_A->r, &mylight_A->g, &mylight_A->b);
    update_light_color(mylight_A);
    
    update_psys(sbox, &psystem);

    if(glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_LEFT)) {
        g_light_selected->x = sbox->mouse_col;
        g_light_selected->y = sbox->mouse_row;

        update_light_pos(sbox, g_light_selected);
    }

    if(sbox->mouse_scroll != 0) {
        
        if(sbox->mouse_scroll < 0) {
            g_light_selected->strength -= 0.1;
        }
        else if(sbox->mouse_scroll > 0) {
            g_light_selected->strength += 0.1;
        }

        update_light_strength(g_light_selected);
    }

    if(glfwGetKey(sbox->win, GLFW_KEY_UP) == GLFW_PRESS) {
        g_light_selected->radius += 0.2;
        update_light_radius(g_light_selected);
    }
    else if(glfwGetKey(sbox->win, GLFW_KEY_DOWN) == GLFW_PRESS) {
        g_light_selected->radius -= 0.2;
        update_light_radius(g_light_selected);
    }
}

void psystem_pupdate_callback(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {
    p->x += p->vel_x * sbox->dt;
    p->y += p->vel_y * sbox->dt;

    p->vel_x += p->acc_x;
    p->vel_y += p->acc_y;

    float rad = lerp(p->lifetime, 4.0, 0.0);

    fillcircle(sbox, p->x, p->y, rad,  p->r, p->g, p->b);
}

void psystem_pdeath_callback(struct sandbox_t* sbox, struct psys_t* psys, struct particle_t* p) {
    p->x = sbox->center_col + randomf(&seed, -10.0, 10.0);
    p->y = sbox->center_row + 30;
   
    p->vel_x = randomf(&seed, -50.0, 50.0);
    p->vel_y = randomf(&seed, 20.0, -80.0);

    float rn_a = randomf(&seed, 0.3, 1.0);
    float rn_b = randomf(&seed, 0.3, 1.0);

    p->r = rn_b - rn_a;
    p->g = rn_a;
    p->b = rn_b;

    p->alive = 1;
    p->lifetime = 0.0;
    p->max_lifetime = randomf(&seed, 0.5, 1.0);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) {
        return;
    }
    switch(key) {
        case GLFW_KEY_Q:
            printf(" Selected Orange light\n");
            g_light_selected = mylight_A;
            break;
        case GLFW_KEY_W:
            printf(" Selected Blue light\n");
            g_light_selected = mylight_B;
            break;
    }
}

void setup(struct sandbox_t* sbox) {
    seed = time(0);
    mylight_A = NULL;
    mylight_B = NULL;

    glfwSetKeyCallback(sbox->win, key_callback);

    init_psys(sbox, &psystem, 100,
            PSYSNOSETUP, psystem_pupdate_callback, psystem_pdeath_callback,
            NULL);

    mylight_A = setup_light(sbox, 
            0, 
            sbox->center_col - 30, /* x */
            sbox->center_row,      /* y */
            0.5,   /* red */
            0.287, /* green */
            0.105, /* blue */
            1.0,   /* strength */
            60.0,  /* radius */
            0.4    /* effect */
            );

    mylight_B = setup_light(sbox, 
            1, 
            sbox->center_col + 30, /* x */
            sbox->center_row + 20, /* y */
            0.105, /* red */
            0.5,   /* green */
            0.55,  /* blue */
            0.5,   /* strength */
            50.0,  /* radius */
            2.0    /* effect */
            );
    
    g_light_selected = mylight_A;


    printf("--> Controls:\n"
            "Select \033[31mOrange light\033[0m press [q]\n"
            "Select \033[34mBlue light\033[0m press [w]\n"
            "Press [ Up/Down Arrow ] to change the selected light Radius\n"
            "Scroll [ Up/Down ] to change the selected light Strength\n");

}


int main() {

    struct sandbox_t sbox;
    if(!init_sandbox(&sbox, 700, 600, "[Sandbox]")) {
        return 1;
    }

    setup(&sbox);

    run_sandbox(&sbox, loop, NULL);

    delete_psys(&psystem);
    free_sandbox(&sbox);

    return 0;
}



