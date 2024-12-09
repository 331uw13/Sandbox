#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "../src/sandbox.h"


static struct texture_t mushroom_tex = { 0 };


void loop(struct sbp_t* sbox, void* ptr) {


    settex(sbox, sbox->center_col, sbox->center_row, &mushroom_tex);

}


int main() {

    struct sbp_t sbox;
    if(!init_sandbox(&sbox, 700, 600, "[Sandbox]")) {
        return 1;
    }

    sb_read_texf("blue_mushroom.tex", &mushroom_tex);

    
    run_sandbox(&sbox, loop, NULL);
    
    sb_delete_tex(&mushroom_tex);
    free_sandbox(&sbox);

    return 0;
}



