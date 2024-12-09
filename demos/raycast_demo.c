#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "../src/sandbox.h"

static int seed = 0;


void draw_raycast_buf(struct sbp_t* sbox) {
    for(int y = 0; y < sbox->max_row; y++) {
        for(int x = 0; x < sbox->max_col; x++) {
            if(sbox->rcbuf[getindexp(sbox, x, y)]) {

                setpixel(sbox, x, y,  0.65, 0.5, 0.5);
            }
        }
    }
}

static float pl_xf;
static float pl_yf;

#define SPEED 80

void loop(struct sbp_t* sbox, void* ptr) {
    float mx = sbox->mouse_col;
    float my = sbox->mouse_row;

    draw_raycast_buf(sbox);

    int pl_x = round((int)pl_xf);
    int pl_y = round((int)pl_yf);

    int hit_x = 0;
    int hit_y = 0;
    int rcid = sb_raycast(sbox,  pl_x, pl_y, mx, my,  &hit_x, &hit_y);


    if(rcid) {
        setline(sbox,  pl_x, pl_y, hit_x, hit_y,  0.8, 0.3, 0.3);
    }
    else {
        setline(sbox,  pl_x, pl_y, mx, my,  0.3, 0.5, 0.3);
    }


    
    if(glfwGetKey(sbox->win, GLFW_KEY_W) == GLFW_PRESS) {
        pl_yf -= SPEED * sbox->dt;
    }
    if(glfwGetKey(sbox->win, GLFW_KEY_S) == GLFW_PRESS) {
        pl_yf += SPEED * sbox->dt;
    }
    if(glfwGetKey(sbox->win, GLFW_KEY_A) == GLFW_PRESS) {
        pl_xf -= SPEED * sbox->dt;
    }
    if(glfwGetKey(sbox->win, GLFW_KEY_D) == GLFW_PRESS) {
        pl_xf += SPEED * sbox->dt;
    }

    setpixel(sbox, pl_x, pl_y,  1.0, 1.0, 1.0);
}

void set_boxtomap(struct sbp_t* sbox, int xpos, int ypos, int width, int height) {
    
    for(int y = ypos; y < (ypos+height); y++) {
        for(int x = xpos; x < (xpos+width); x++) {
            sb_rcbuf_setid(sbox, x, y, 1);
        }
    }

}

void init_map(struct sbp_t* sbox) {
    
    sb_init_rcbuf(sbox);
    
    set_boxtomap(sbox, 30, 30, 20, 30);
    set_boxtomap(sbox, 120, 50, 50, 30);
    set_boxtomap(sbox, 80, 100, 10, 30);
    set_boxtomap(sbox, 50, 70, 10, 10);


}



int main() {

    struct sbp_t sbox;
    if(!init_sandbox(&sbox, 700, 600, "[Sandbox]")) {
        return 1;
    }

    pl_xf = sbox.center_col;
    pl_yf = sbox.center_row;

    seed = time(0);
    init_map(&sbox);
        
    run_sandbox(&sbox, loop, NULL);
    free_sandbox(&sbox);

    return 0;
}



