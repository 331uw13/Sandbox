#include <stdio.h>
#include <math.h>

#include "../../src/sandbox.h"

#define TEX_MAX_WIDTH 64
#define TEX_MAX_HEIGHT 64

#define COLOR_PALETTE_MAX 16

// x position where cursor is detected to be on color palette
#define MOUSE_ON_COLORPALETTE_TRESHOLD 10 


static int g_grid_zoom;
static int g_grid_x;
static int g_grid_y;

static int g_mouse_prev_col;
static int g_mouse_prev_row;

static int g_show_grid;


struct texpixel_t {
    float red;
    float grn;
    float blu;
    int x;
    int y;
    int active;
};

struct color_t {
    float red;
    float grn;
    float blu;
};


static struct texpixel_t 
g_texdata[TEX_MAX_WIDTH * TEX_MAX_HEIGHT] = { 0 };

static struct color_t
g_colorpalette[COLOR_PALETTE_MAX] = { 0 };


void loop(struct sandbox_t* sbox, void* ptr) {

    g_grid_zoom += sbox->mouse_scroll;
    g_grid_zoom = CLAMP(g_grid_zoom, 1, 10);

    int drag_cursor_mod = 1;

    if(glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_MIDDLE)
            == GLFW_PRESS) {

        g_grid_x += (sbox->mouse_col - g_mouse_prev_col);
        g_grid_y += (sbox->mouse_row - g_mouse_prev_row);

        drag_cursor_mod = 2;
    }

    g_mouse_prev_col = sbox->mouse_col;
    g_mouse_prev_row = sbox->mouse_row;

    const int texhalfwidth = (TEX_MAX_WIDTH * g_grid_zoom) / 2;
    const int texhalfheight = (TEX_MAX_HEIGHT * g_grid_zoom) / 2;

    const int grid_pos_x = g_grid_x - texhalfwidth;
    const int grid_pos_y = g_grid_y - texhalfheight;

    // draw grid
    for(int gy = 0; gy < TEX_MAX_HEIGHT; gy++) {
        for(int gx = 0; gx < TEX_MAX_WIDTH; gx++) {
 

            float r = 0.0;
            float g = 0.0;
            float b = 0.0;


            struct texpixel_t* tp = &g_texdata[gy * TEX_MAX_WIDTH + gx];

            if(tp->active) {
                r = tp->red;
                g = tp->grn;
                b = tp->blu;
            }
            else {
                float c = (g_show_grid) ? (((gx+gy) % 2) ? 0.035 : 0.055) : 0.0;
                r = c;
                g = c;
                b = c;
            }

            setbox(sbox,
                    gx*g_grid_zoom + grid_pos_x,
                    gy*g_grid_zoom + grid_pos_y,
                    g_grid_zoom, 
                    g_grid_zoom,
                    r, g, b
                    );
        }
    }

    // draw palette
    {
        const int palette_height = COLOR_PALETTE_MAX * 7;
        const int palette_y_start = (sbox->max_row / 2) - (palette_height / 2);
        
        // side lines
        setline(sbox, 2, palette_y_start, 2, palette_y_start+palette_height-1,
                0.1,0.1,0.1);
        
        setline(sbox, 2+7, palette_y_start, 2+7, palette_y_start+palette_height-1,
                0.1,0.1,0.1);
        
        // bottom line
        setline(sbox, 3, palette_y_start+palette_height-1, 2+7, palette_y_start+palette_height-1,
                0.1,0.1,0.1);


        // boxes
        for(int i = 0; i < COLOR_PALETTE_MAX; i++) {
            
            int x = 3;
            int y = i * 7 + palette_y_start;


            setbox(sbox, x, y, 6, 6, 
                    g_colorpalette[i].red,
                    g_colorpalette[i].grn,
                    g_colorpalette[i].blu);
            setline(sbox, x, y-1, x+6, y-1,  0.1,0.1,0.1);
        }
    }


    // handle mouse stuff
    //
    {
        if(glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

            long int x = ((sbox->mouse_col - g_grid_x + texhalfwidth) / g_grid_zoom);
            long int y = ((sbox->mouse_row - g_grid_y + texhalfheight) / g_grid_zoom);

            if((x >= 0) && (y >= 0) && (x < TEX_MAX_WIDTH) && (y < TEX_MAX_HEIGHT)) {
                size_t texdata_index =
                    (y * TEX_MAX_WIDTH + x);

                struct texpixel_t* tp = &g_texdata[texdata_index];
                tp->x = x;
                tp->y = y;

                tp->active = 1;
                tp->red = 1.0;
                tp->grn = 0.2;
                tp->blu = 0.8;

            }

        }
    }


    // draw cursor

    setpixel(sbox, 
            sbox->mouse_col, sbox->mouse_row,
            0.3, 1.0, 0.3);

    setpixel(sbox, 
            sbox->mouse_col+drag_cursor_mod, sbox->mouse_row,
            0.25, 0.66, 0.25);

    setpixel(sbox, 
            sbox->mouse_col, sbox->mouse_row+drag_cursor_mod,
            0.25, 0.66, 0.25);
}

void dump_texture_data_stdout() {
    
    for(int y = 0; y < TEX_MAX_HEIGHT; y++) {
        for(int x = 0; x < TEX_MAX_WIDTH; x++) {
            size_t i = y * TEX_MAX_WIDTH + x;
            if(!g_texdata[i].active) { continue; }
            printf("%i | XY(%i, %i) | RGB(%0.2f, %0.2f, %0.2f)\n",
                    i,
                    g_texdata[i].x, g_texdata[i].y,
                    g_texdata[i].red, g_texdata[i].grn, g_texdata[i].blu);
        } 
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) { return; }

    struct sandbox_t* sbox = (struct sandbox_t*)glfwGetWindowUserPointer(window);
    if(!sbox) {
        fprintf(stderr, "%s: glfwGetWindowUserPointer failed!\n",
                __func__);
        return;
    }

    if(mods != GLFW_MOD_CONTROL) { 
        switch(key) {

            case GLFW_KEY_K:
                dump_texture_data_stdout();
                break;


            case GLFW_KEY_G:
                g_show_grid = !g_show_grid;
                break;
        }
    }
    else { 
        switch(key) {

            case GLFW_KEY_N:
                if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)
                        == GLFW_PRESS)
                {
                    for(int i = 0; i < (TEX_MAX_WIDTH * TEX_MAX_HEIGHT); i++) {
                        struct texpixel_t* tp = &g_texdata[i];
                        tp->red = 0.0;
                        tp->grn = 0.0;
                        tp->x = 0;
                        tp->y = 0;
                        tp->active = 0;
                    }
                }
                break;
        
            case GLFW_KEY_TAB:
                g_grid_x = sbox->center_col;
                g_grid_y = sbox->center_row;
                break;
        }

    }
}

void print_control_desc(const char* control, const char* name) {
    printf(" [\033[35m%s\033[0m]: \033[90m%s\033[0m\n", control, name);
}

void print_controls() {

    printf("---( Texture editor controls )---\n");
    
    print_control_desc("CONTROL + TAB", "Center grid position.");
    print_control_desc("MOUSE MID BUTTON + DRAG", "Drag grid.");
    print_control_desc("MOUSE SCROLL", "Zoom.");
    print_control_desc("HOLD MOUSE RIGHT then CONTROL + N", "Delete data.");
    print_control_desc("G", "Toggle grid background.");
    print_control_desc("C", "Modify selected color.");

    printf("---------------------------------\n");


}

void init_colorpalette() {
    
    for(int i = 0; i < COLOR_PALETTE_MAX; i++) {
        
        rainbow_palette(sin((float)i*0.07),
                &g_colorpalette[i].red,
                &g_colorpalette[i].grn,
                &g_colorpalette[i].blu
                );

    }

}


int main() {


    struct sandbox_t sbox;
    if(!init_sandbox(&sbox, 900, 700, "[TexEdit]")) {
        return 1;
    }

    glfwSetWindowUserPointer(sbox.win, &sbox);

    print_controls();
    init_colorpalette();

    g_grid_zoom = 2;
    g_grid_x = sbox.center_col;
    g_grid_y = sbox.center_row;
    g_mouse_prev_col = 0;
    g_mouse_prev_row = 0;
    g_show_grid = 1;

    glfwSetKeyCallback(sbox.win, key_callback);

    show_cursor(&sbox, 0);
    sbox.render_mode = RENDERMODE_POLL_EVENTS;

    run_sandbox(&sbox, loop, NULL);
    free_sandbox(&sbox);

    return 0;
}





