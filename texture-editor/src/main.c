#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../../src/sandbox.h"

static long int TEX_MAX_COLUMN;
static long int TEX_MAX_ROW;

#define COLORPALETTE_MAX 20
#define COLORPALETTE_BOX 6

// x position where cursor is detected to be on color palette
#define MOUSE_ON_COLORPALETTE_TRESHOLD 16



// TODO: rewrite this whole thing someday.
// ----


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

struct slider_t {
    int x;
    int y;
    int width;
    float* valueptr;
    float min;
    float max;
    float r; 
    float g;
    float b;
};

static int g_grid_zoom;
static int g_grid_x;
static int g_grid_y;

static int g_mouse_prev_col;
static int g_mouse_prev_row;

static int g_show_grid;
static int g_mode;
static int g_erase_size;
static int g_draw_size;
static int g_color_modify_visible;

#define MODE_DRAW 0
#define MODE_ERASE 1


static struct texpixel_t* g_texdata;

static struct color_t
g_colorpalette[COLORPALETTE_MAX] = { 0 };

static long int g_current_colorp_index;
static struct color_t* g_current_color;
static struct color_t g_copied_color = { 0 };

#define MAX_SLIDERS 3
static struct slider_t g_sliders[MAX_SLIDERS];
static size_t g_num_sliders;
static long int g_current_slider_index;

#define TEXFILEPATH_MAX 255
static char g_texfilepath[TEXFILEPATH_MAX] = { 0 };
static size_t g_texfilepath_size;

#define SLIDER_SX 30
#define SLIDER_SY 5
#define SLIDER_WIDTH 50
#define SLIDER_HEIGHT 7



void add_slider(int x, int y, int width, float* valueptr, float min, float max,
        float r, float g, float b) {
    
    if(g_num_sliders+1 > MAX_SLIDERS) {
        fprintf(stderr, "%s: max sliders reached.\n", __func__);
        return;
    }

    g_sliders[g_num_sliders] = (struct slider_t) {
        .x = x,
        .y = y,
        .width = width,
        .valueptr = valueptr,
        .min = min,
        .max = max,
        .r = r,
        .g = g,
        .b = b
    };

    g_num_sliders++;
}


void init_sliders() {

    // RED
    add_slider(
            SLIDER_SX,
            SLIDER_SY,
            SLIDER_WIDTH,
            &g_current_color->red, 0.0, 1.0,
            1.0, 0.5, 0.5
            );

    // GREEN
    add_slider(
            SLIDER_SX,
            SLIDER_SY + SLIDER_HEIGHT+1,
            SLIDER_WIDTH,
            &g_current_color->grn, 0.0, 1.0,
            0.5, 1.0, 0.5
            );

    // BLUE
    add_slider(
            SLIDER_SX,
            SLIDER_SY + (SLIDER_HEIGHT+1) * 2,
            SLIDER_WIDTH,
            &g_current_color->blu, 0.0, 1.0,
            0.5, 0.5, 1.0
            );
}


void update_sliders(struct sbp_t* sbox) {
    
    struct slider_t* s = NULL;
    int mouse_left_hold = glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_LEFT);

    if(!mouse_left_hold) {
        // select none of the sliders to be currently active.
        g_current_slider_index = -1; 
    }

    for(size_t i = 0; i < g_num_sliders; i++) {
        s = &g_sliders[i];
        if(!s) { continue; }

        int mouse_on = 
            (sbox->mouse_col > s->x && sbox->mouse_col < s->x + s->width)
        &&  (sbox->mouse_row > s->y && sbox->mouse_row < s->y + SLIDER_HEIGHT);
        
        if(g_current_slider_index == -1 && mouse_on && mouse_left_hold) {
            g_current_slider_index = i;
        }


        setbox(sbox, s->x, s->y, s->width+1, SLIDER_HEIGHT,
                s->r*0.25, s->g*0.25, s->b*0.25);

        float value_x = map(*s->valueptr, 0.0, 1.0, s->x, s->x+s->width);
        setline(sbox, value_x, s->y, value_x, s->y+SLIDER_HEIGHT, s->r,s->g,s->b);

    }

    if(g_current_slider_index >= 0) {
        s = &g_sliders[g_current_slider_index];

        *s->valueptr = map(sbox->mouse_col - s->x, 0, s->width, 0.0, 1.0);
        *s->valueptr = CLAMP(*s->valueptr, 0.0, 1.0);
    }
}


void get_texpixels_from_circle(
        int ix, int iy,
        int radius,
        void(*callback)(struct texpixel_t*))
{
    if(!callback) {
        return;
    }

    if(ix * iy < 0) {
        return;
    }

    if(radius == 1) {
        size_t tpindx = (size_t)(iy * TEX_MAX_COLUMN + ix);
        if(tpindx < (size_t)(TEX_MAX_COLUMN * TEX_MAX_ROW)) {
            callback(&g_texdata[tpindx]);
        }
        return;
    }




    int ystart = iy - radius;
    int yend   = iy + radius;
    
    int xstart = ix - radius;
    int xend   = ix + radius;
    

    for(int y = ystart; y <= yend; y++) {
        for(int x = xstart; x <= xend; x++) {
  
            if(x*y < 0) { continue; }
            if(x >= TEX_MAX_COLUMN) { continue; }

            float dst = vdistance(x+0.5, y+0.5, ix, iy);

            if(dst > radius) {
                continue;
            }

            size_t tpindx = (size_t)(y * TEX_MAX_COLUMN + x);
            if(tpindx > (size_t)(TEX_MAX_COLUMN * TEX_MAX_ROW)) {
                continue;
            }

            callback(&g_texdata[tpindx]);
        }
    }
}

void erase_pixels_callback(struct texpixel_t* tp) {
    if(tp) {
        tp->active = 0;
        tp->red = 0.0;
        tp->grn = 0.0;
        tp->blu = 0.0;
    }
}

void draw_pixels_callback(struct texpixel_t* tp) {
    if(tp) {
        struct color_t* color = &g_colorpalette[g_current_colorp_index];
        tp->active = 1;
        tp->red = color->red;
        tp->grn = color->grn;
        tp->blu = color->blu;
    }
}



void handle_user_grid_click(int x, int y) {

    if(g_color_modify_visible) {
        return;
    }

    if(x * y < 0) {
        return;
    }

    size_t texdata_index =
        (y * TEX_MAX_COLUMN + x);

    if(texdata_index > (size_t)(TEX_MAX_COLUMN * TEX_MAX_ROW)) {
        return;
    }

    struct texpixel_t* tp = &g_texdata[texdata_index];
    tp->x = x;
    tp->y = y;


    if(g_mode == MODE_DRAW) {
        get_texpixels_from_circle(x, y, g_draw_size, draw_pixels_callback);
    }
    else if(g_mode == MODE_ERASE) {
        get_texpixels_from_circle(x, y, g_erase_size, erase_pixels_callback);
    }

}

void color_grid_line(int x0, int y0, int x1, int y1) {
    int width = x1-x0;
    int height = y1-y0;
    int dx0 = 0;
    int dy0 = 0;
    int dx1 = 0;
    int dy1 = 0;

    dx1 = dx0 = (width < 0) ? -1 : 1;
    dy0 = (height < 0) ? -1 : 1;

    int aw = abs(width);
    int ah = abs(height);
    int longest = aw;
    int shortest = ah;

    if(longest < shortest) {
        longest = ah;
        shortest = aw;
        dy1 = (height < 0) ? -1 : 1;
        dx1 = 0;
    }

    int numerator = longest >> 1;

    for(int i = 0; i < longest; i++) {

        handle_user_grid_click(x0, y0);

        numerator += shortest;
        if(numerator > longest) {
            numerator -= longest;
            x0 += dx0;
            y0 += dy0;
        }
        else {
            x0 += dx1;
            y0 += dy1;
        }
    }
}

void update_sliders_valueptrs() {
    if(g_current_colorp_index >= COLORPALETTE_MAX) {
        return;
    }

    g_current_color = &g_colorpalette[g_current_colorp_index];
    g_sliders[0].valueptr = &g_current_color->red;
    g_sliders[1].valueptr = &g_current_color->grn;
    g_sliders[2].valueptr = &g_current_color->blu;
}

void handle_colorpalette_click(struct sbp_t* sbox, 
        int colorpalette_y, int colorpalette_height) {

    size_t index = (sbox->mouse_row - colorpalette_y) / (COLORPALETTE_BOX+1);

    if(index >= COLORPALETTE_MAX) {
        fprintf(stderr, "%s: invalid colorpalette index %li\n",
                __func__, index);
        return;
    }

    g_current_colorp_index = index;
    update_sliders_valueptrs();
}


void loop(struct sbp_t* sbox, void* ptr) {


    if(glfwGetKey(sbox->win, GLFW_KEY_V) || g_color_modify_visible) {
        
        if(sbox->mouse_scroll > 0) {
            g_current_colorp_index--;
            g_current_colorp_index = CLAMP(g_current_colorp_index, 0, COLORPALETTE_MAX-1);
        } 
        else if(sbox->mouse_scroll < 0) {
            g_current_colorp_index++;
            g_current_colorp_index = CLAMP(g_current_colorp_index, 0, COLORPALETTE_MAX-1);
        }
        update_sliders_valueptrs();
    }
    else {
        g_grid_zoom += sbox->mouse_scroll;
        g_grid_zoom = CLAMP(g_grid_zoom, 1, 10);

    }
    int drag_cursor_mod = 1;

    if(glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_MIDDLE)
            == GLFW_PRESS) {

        g_grid_x += (sbox->mouse_col - g_mouse_prev_col);
        g_grid_y += (sbox->mouse_row - g_mouse_prev_row);

        drag_cursor_mod = 2;
    }

    // handle color dim/brighten here because the keys must be able to be held down
    // thats not possible in key_callback
    // multiply with frame delta time so its same speed not depend on processor or the this program.
    if(g_color_modify_visible) {
        const float dimdtmult = 0.4;
        if(glfwGetKey(sbox->win, GLFW_KEY_1) == GLFW_PRESS) {
            g_current_color->red = CLAMP(g_current_color->red - dimdtmult * sbox->dt, 0.0, 1.0);
            g_current_color->grn = CLAMP(g_current_color->grn - dimdtmult * sbox->dt, 0.0, 1.0);
            g_current_color->blu = CLAMP(g_current_color->blu - dimdtmult * sbox->dt, 0.0, 1.0);

        }
        else if(glfwGetKey(sbox->win, GLFW_KEY_2) == GLFW_PRESS) {
            g_current_color->red = CLAMP(g_current_color->red + dimdtmult * sbox->dt, 0.0, 1.0);
            g_current_color->grn = CLAMP(g_current_color->grn + dimdtmult * sbox->dt, 0.0, 1.0);
            g_current_color->blu = CLAMP(g_current_color->blu + dimdtmult * sbox->dt, 0.0, 1.0);
        }
    }

    const int texhalfwidth = (TEX_MAX_COLUMN * g_grid_zoom) / 2;
    const int texhalfheight = (TEX_MAX_ROW * g_grid_zoom) / 2;

    const int grid_pos_x = g_grid_x - texhalfwidth;
    const int grid_pos_y = g_grid_y - texhalfheight;

    // draw grid
    for(int gy = 0; gy < TEX_MAX_ROW; gy++) {
        for(int gx = 0; gx < TEX_MAX_COLUMN; gx++) {
 

            float r = 0.0;
            float g = 0.0;
            float b = 0.0;


            struct texpixel_t* tp = &g_texdata[gy * TEX_MAX_COLUMN + gx];

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
    const int palette_height = COLORPALETTE_MAX * 7;
    const int palette_y_start = (sbox->max_row / 2) - (palette_height / 2);
    {
        
        const int widthplus1 = COLORPALETTE_BOX+1;

        // side lines
        //
        // left
        setline(sbox,
                2, palette_y_start,
                2, palette_y_start+palette_height-1,
                0.1,0.1,0.1);
        // right
        setline(sbox, 
                2+widthplus1, palette_y_start,
                2+widthplus1, palette_y_start+palette_height-1,
                0.1,0.1,0.1);
        
        // bottom line
        setline(sbox,
                3, palette_y_start+palette_height-1,
                2+widthplus1, palette_y_start+palette_height-1,
                0.1,0.1,0.1);


        // boxes
        for(int i = 0; i < COLORPALETTE_MAX; i++) {
            
            int x = 3;
            int y = i * widthplus1 + palette_y_start;


            setbox(sbox, x, y, COLORPALETTE_BOX, COLORPALETTE_BOX, 
                    g_colorpalette[i].red,
                    g_colorpalette[i].grn,
                    g_colorpalette[i].blu);
            setline(sbox, x, y-1, x+COLORPALETTE_BOX, y-1,  0.1,0.1,0.1);
        }

        // indicate selected color

        setbox(sbox,
                4+COLORPALETTE_BOX,
                2+palette_y_start + (g_current_colorp_index * (COLORPALETTE_BOX+1)),
                2,2,
                0.3, 1.0, 0.3
                );

    }


    // handle mouse stuff on grid and colorpalette
    //
    {
        if(glfwGetMouseButton(sbox->win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

            int mouse_on_colorpalette =
                (sbox->mouse_col < MOUSE_ON_COLORPALETTE_TRESHOLD)
                && (sbox->mouse_row > palette_y_start)
                && (sbox->mouse_row < (palette_y_start+palette_height));


            
            if(!mouse_on_colorpalette) {
                
                long int p_x = ((g_mouse_prev_col - g_grid_x + texhalfwidth) / g_grid_zoom);
                long int p_y = ((g_mouse_prev_row - g_grid_y + texhalfheight) / g_grid_zoom);
                
                long int x = ((sbox->mouse_col - g_grid_x + texhalfwidth) / g_grid_zoom);
                long int y = ((sbox->mouse_row - g_grid_y + texhalfheight) / g_grid_zoom);
                
                if((x >= 0) && (y >= 0) && (x < TEX_MAX_COLUMN) && (y < TEX_MAX_ROW)
                && (p_x >= 0) && (p_y >= 0) && (p_x < TEX_MAX_COLUMN) && (p_y < TEX_MAX_ROW)) {
                    if(x == p_x && y == p_y) {
                        handle_user_grid_click(x, y);
                    }
                    else {
                        color_grid_line(p_x, p_y, x, y);
                    }
                }
            }
            else {
                handle_colorpalette_click(sbox, palette_y_start, palette_height);
            }


        }
    }

    if(g_color_modify_visible) {
        update_sliders(sbox);
    }

    if(g_mode == MODE_DRAW) {
        struct color_t* color = &g_colorpalette[g_current_colorp_index];
        fillcircle(sbox, 
                5+g_draw_size/2, 
                5+g_draw_size/2, g_draw_size,
                color->red, color->grn, color->blu
                );

    }
    else if(g_mode == MODE_ERASE) {
       
        // top horizontal line
        setline(sbox, 
                3, 3,
                3+5, 3,
                1.0, 0.7, 0.7);
        
        // left vertical line
        setline(sbox, 
                3, 3,
                3, 3+7,
                1.0, 0.7, 0.7);


        // middle horizontal line
        setline(sbox, 
                4, 4+2,
                4+3, 4+2,
                1.0, 0.7, 0.7);
 
        // bottom horizontal line
        setline(sbox, 
                3, 3+6,
                3+5, 3+6,
                1.0, 0.7, 0.7);

        // erase size.
        fillcircle(sbox, 
                13+g_erase_size/2, 
                5+g_erase_size/2, g_erase_size,
                1.0, 0.6, 0.5);
    }

    g_mouse_prev_col = sbox->mouse_col;
    g_mouse_prev_row = sbox->mouse_row;

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
    
    for(int y = 0; y < TEX_MAX_ROW; y++) {
        for(int x = 0; x < TEX_MAX_COLUMN; x++) {
            size_t i = y * TEX_MAX_COLUMN + x;
            if(!g_texdata[i].active) { continue; }
            printf("%li | XY(%i, %i) | RGB(%0.2f, %0.2f, %0.2f)\n",
                    i,
                    g_texdata[i].x, g_texdata[i].y,
                    g_texdata[i].red, g_texdata[i].grn, g_texdata[i].blu);
        } 
    }
}


void write_texdata();

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) { return; }

    struct sbp_t* sbox = (struct sbp_t*)glfwGetWindowUserPointer(window);
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

            case GLFW_KEY_E:
                g_mode = !g_mode;
                break;
        
            case GLFW_KEY_D:
                if(g_mode == MODE_ERASE) {
                    g_erase_size--;
                    g_erase_size = CLAMP(g_erase_size, 1, 6);
                }
                else if(g_mode == MODE_DRAW) {
                    g_draw_size--;
                    g_draw_size = CLAMP(g_draw_size, 1, 6);
                }
                break;
        
            case GLFW_KEY_F:
                if(g_mode == MODE_ERASE) {
                    g_erase_size++;
                    g_erase_size = CLAMP(g_erase_size, 1, 6);
                }
                else if(g_mode == MODE_DRAW) {
                    g_draw_size++;
                    g_draw_size = CLAMP(g_draw_size, 1, 6);
                }
                break;

            case GLFW_KEY_C:
                g_color_modify_visible = !g_color_modify_visible;
                break;

            case GLFW_KEY_Z:
                if(g_color_modify_visible) {
                    g_copied_color = (struct color_t) {
                        .red = g_current_color->red,
                        .grn = g_current_color->grn,
                        .blu = g_current_color->blu,
                    };
                }
                break;

            case GLFW_KEY_X:
                if(g_color_modify_visible) {
                    g_current_color->red = g_copied_color.red;
                    g_current_color->grn = g_copied_color.grn;
                    g_current_color->blu = g_copied_color.blu;
                }
                break;
        }
    }
    else { 
        switch(key) {

            case GLFW_KEY_S:
                write_texdata();
                break;

            case GLFW_KEY_N:
                if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)
                        == GLFW_PRESS)
                {
                    for(int i = 0; i < (TEX_MAX_COLUMN * TEX_MAX_ROW); i++) {
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
void print_control_desc_sub(const char* control, const char* name) {
    printf("   |_[\033[35m%s\033[0m]: \033[90m%s\033[0m\n", control, name);
}

void print_controls() {

    printf("---( TexEdit controls )---\n");
    
    print_control_desc("CONTROL + TAB", "Center grid position.");
    print_control_desc("MOUSE MID BUTTON + DRAG", "Drag grid.");
    print_control_desc("MOUSE SCROLL", "Zoom.");
    print_control_desc("HOLD MOUSE RIGHT then CONTROL + N", "Delete data.");
    print_control_desc("G", "Toggle grid background.");
    print_control_desc("E", "Toggle erase mode.");
    print_control_desc("D", "Decrease erase/draw size");
    print_control_desc("F", "Increase erase/draw size");
    print_control_desc("C", "Modify selected color.");
    print_control_desc_sub("Z", "Copy selected color.");
    print_control_desc_sub("X", "Paste selected color.");
    print_control_desc_sub("1", "Dim selected color.");
    print_control_desc_sub("2", "Brighten selected color.");
    print_control_desc("MOUSE SCROLL + V", "Select color from palette.");
    print_control_desc("CONTROL + S", "Save.");

    printf("---------------------------------\n");
}


void print_usage_desc(const char* what, const char* desc) {
    printf(" \033[36m%s \033[0m%s\n", what, desc);
}

void print_usage(const char* firstarg) {
    printf("\n");
    
    print_usage_desc(firstarg, "(Existing Filepath)");
    print_usage_desc(firstarg, "(New Filepath) (MaxColumns) (MaxRows)");

    printf("\n");

}


void init_colorpalette() {
    for(int i = 0; i < COLORPALETTE_MAX; i++) {
        
        rainbow_palette(sin((float)i*0.07),
                &g_colorpalette[i].red,
                &g_colorpalette[i].grn,
                &g_colorpalette[i].blu
                );

    }
    g_current_color = &g_colorpalette[0];
}

/*

   texture file structure:


   first 4 bytes: texture width.
   second 4 bytes: texture height.

   1 segment contains (in order)
    - pixel x position
    - pixel y position
    - red value            rgb is from 0 to 255
    - green value          scale it to 0.0 to 1.0 when reading texture.
    - blue value

    these segments are next to each other until end of file


    struct texpixel_t {
        float red;
        float grn;
        float blu;
        int x;
        int y;
        int active;
    };

*/

void write_texdata() {
   
    int fd = open(g_texfilepath, O_WRONLY | O_APPEND);
    if(fd == -1) {
        perror("open");
        return;
    }

    size_t texsize = (size_t)(TEX_MAX_COLUMN * TEX_MAX_ROW);    
    int buf[SB_TEXTURE_SEGMENT_SIZE] = { 0 };

    for(size_t i = 0; i < texsize; i++) {
        struct texpixel_t* tp = &g_texdata[i];
        if(!tp->active) { continue; }


    }



    close(fd);
    printf("saved.\n");
}


int init_from_existing_file() {
    int result = 0;

    if(access(g_texfilepath, F_OK)) {
        fprintf(stderr, "File '%s' doesnt exist\n", g_texfilepath);
        goto error;
    }




    result = 1;
error:
    return result;
}

int init_new_file(int maxcol, int maxrow) {
    int result = 0;

    if(!access(g_texfilepath, F_OK)) {
        fprintf(stderr, "File '%s' exists. cant overwrite.\n", g_texfilepath);
        goto error;
    }

    TEX_MAX_COLUMN = maxcol;
    TEX_MAX_ROW = maxrow;

    result = 1;

error:
    return result;
}

#define RET_SUCCESS 0
#define RET_FAILURE 1

int main(int argc, char** argv) {

    if((argc <= 1) || (argc == 3) || (argc > 4)) {
        printf("\nInvalid number of arguments\n");
        print_usage(argv[0]);
        return RET_FAILURE;
    }

    g_texfilepath_size = 0;
    g_texfilepath_size = strlen(argv[1]);
    if(g_texfilepath_size >= TEXFILEPATH_MAX) {
        fprintf(stderr, "Filepath length too big (%li)\n",
                g_texfilepath_size);
        return RET_FAILURE;
    }

    memmove(g_texfilepath, argv[1], g_texfilepath_size);

    if(argc == 2) {
        if(!init_from_existing_file()) {
            print_usage(argv[0]);
            return RET_FAILURE;
        }
        printf("\033[32m +> \033[0mEditing Existing file '%s'\n",
                g_texfilepath);
    }
    else if(argc == 4) {
        const int cols = atoi(argv[2]);
        const int rows = atoi(argv[3]);

        if(cols <= 1) {
            fprintf(stderr, "\n\033[31m Invalid (MaxColumn)\033[0m\n");
            print_usage(argv[0]);
            return RET_FAILURE;
        }
        if(rows <= 1) {
            fprintf(stderr, "\n\033[31m Invalid (MaxRow)\033[0m\n");
            print_usage(argv[0]);
            return RET_FAILURE;
        }

        if(!init_new_file(cols, rows)) {
            print_usage(argv[0]);
            return RET_FAILURE;
        }
    
        printf("\033[32m +> \033[0mEditing New file '%s' %ix%i\n",
                g_texfilepath, cols, rows);
    }


    size_t texdatasize = sizeof *g_texdata * (TEX_MAX_COLUMN * TEX_MAX_ROW);
    g_texdata = NULL;
    g_texdata = malloc(texdatasize);

    if(!g_texdata) {
        perror("malloc");
        fprintf(stderr, "Failed to allocate memory for texture data. size too big?\n");
        return RET_FAILURE;
    }


    struct sbp_t sbox;
    if(!init_sandbox(&sbox, 1200, 900, "Texture Editor")) {
        return RET_FAILURE;
    }

    glfwSetWindowUserPointer(sbox.win, &sbox);
    g_mode = MODE_DRAW;
    g_color_modify_visible = 0;
    g_erase_size = 1;
    g_draw_size = 1;
    g_grid_zoom = 1;
    g_grid_x = sbox.center_col;
    g_grid_y = sbox.center_row;
    g_mouse_prev_col = 0;
    g_mouse_prev_row = 0;
    g_show_grid = 1;
    g_current_colorp_index = 0;
    g_num_sliders = 0;
    g_current_slider_index = -1;

    print_controls();
    init_colorpalette();
    init_sliders();


    glfwSetKeyCallback(sbox.win, key_callback);

    show_cursor(&sbox, 0);
    sbox.render_mode = RENDERMODE_POLL_EVENTS;

    run_sandbox(&sbox, loop, NULL);

    free(g_texdata);
    free_sandbox(&sbox);

    printf("exit.\n");
    return RET_SUCCESS;
}


