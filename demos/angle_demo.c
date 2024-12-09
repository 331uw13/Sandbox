#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "../src/sandbox.h"


static int mscroll = 0;


void loop(struct sbp_t* sbox, void* ptr) {
    mscroll += sbox->mouse_scroll;
    float mx = sbox->mouse_x;
    float my = sbox->mouse_y;

    setpixel(sbox, sbox->center_col, sbox->center_row,  0.1, 0.2, 0.1);

    float ang = vangle(mx, my, sbox->center_col, sbox->center_row);
    ang *= _PI_R;


    float len = mscroll;
    float x = mx + len * cos(ang);
    float y = my + len * sin(ang);



    setline(sbox,
            mx, my,
            x, y,

            0.3, 1.0, 0.3

            );


}


int main() {

    struct sbp_t sbox;
    if(!init_sandbox(&sbox, 700, 600, "[Sandbox]")) {
        return 1;
    }

    mscroll = 15;
    
    run_sandbox(&sbox, loop, NULL);
    free_sandbox(&sbox);

    return 0;
}



