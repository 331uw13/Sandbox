#include "ewglu.h"
#include "lights.h"
#include "texture.h"

#define PIXELSIZE 4

#define F_CLEARSCREEN 1
#define F_NOMOUSEPOS  2


// NOTE: RGB's are from 0.0 to 1.0


/*

   TODO. (not in any order)

    - function to return pixels nearby for ray trace?

    - textures

    - animations

    - pixel blending

    - 2D matrixes?



*/

#define RENDERMODE_POLL_EVENTS 1
#define RENDERMODE_WAIT_EVENTS 2


// uniform location index
#define EFFECT_UNILOC_INUMLIGHTS 0
#define EFFECT_UNILOC_ILIGHTS 1
#define EFFECT_UNILOC_ITIME 2
#define EFFECTSHADER_MAX_UNILOCS 4

#define _PI   3.1415
#define _2PI  6.2831
#define _PI_R 0.0174

#define GLSL_VERSION "#version 430\n"
#include "shaders.h"

#define CLAMP(t, min, max) ((t < min) ? min : (t > max) ? max : t)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

struct effect_t {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int ubo;
    unsigned int tex;
    unsigned int shader;
    
    // store uniform locations here
    // so they dont need to be asked
    // where they are over and over again.
    int unilocs[EFFECTSHADER_MAX_UNILOCS];
};

struct pixels_t {
    unsigned int vao;
    unsigned int vbo;
    unsigned int shader;
    float* buffer;
    size_t buffer_sizeb;
    size_t num_maxpixels;
};

struct sbp_t {
    GLFWwindow* win;
    int running;

    int max_col;
    int max_row;
    int center_row;
    int center_col;

    int win_width;
    int win_height;


    struct pixels_t pixels;
    struct effect_t effect;

    double time;
    double dt;  // delta time (previous frame time)
    double mouse_col;
    double mouse_row;

    int render_mode; // set the render mode to RENDERMODE_WAIT_EVENTS
                     // for programs that dont need to be always re-rendered.
                     // RENDERMODE_POLL_EVENTS (default) is not waiting
                     //                         for user event before starting new frame.

    // these mouse events are cleared at end of frame.
    // they do not stay at value 1 if the button is held down.
    // you can use glfwMouseButton() if you need it to stay.
    int mouse_left_down;
    int mouse_right_down;
    int mouse_middle_down;
    int mouse_scroll;

    // buffer for raycast. 
    // to use it first call sb_init_rcbuf() to initialize it.
    // it will be allocated (num_maxpixels * sizeof *rcbuf) bytes of memory
    // and rcbuf_avail is set to 1 if success.
    // call free_rcbuf after use, 
    // it is also called when run_sandbox exits the loop
    // you can set the ID with 'sb_rcbuf_setid(sanbox_t*, x, y, id)' 
    // zero is treated as 'air'
    // and then the function 'raycast(sbp_t*, start_x, start_y, end_x, end_y)'
    // will return the ID if it hit anything non zero.
    int* rcbuf;
    int rcbuf_avail;

    struct light_t lights[MAX_LIGHTS];
    size_t num_lights;


    int flags;
};


int init_sandbox(struct sbp_t* sbox, int width, int height, const char* window_name);
void run_sandbox(struct sbp_t* sbox,
        void(*loop_callback)(struct sbp_t* sbox, void*), void* userptr);
void free_sandbox(struct sbp_t* sbox);

// ---- UTILITY FUNCTIONS ----

void  sb_rainbow_palette(float t, float* r, float* g, float* b);
float vdistance(float x0, float y0, float x1, float y1);
float vdot(float x0, float y0,  float x1, float y1);
float vlength(float x, float y);
float vangle(float x0, float y0, float x1, float y1);
size_t sb_getindexp(struct sbp_t* sbox, int x, int y);
void sb_show_cursor(struct sbp_t* sbox, int mode);


//  ---- RAYCAST FUNCTIONS ----

#define RAYCAST_AIRID 0


int sb_init_rcbuf(struct sbp_t* sbox);
void sb_free_rcbuf(struct sbp_t* sbox);

void sb_rcbuf_setid(struct sbp_t* sbox, int x, int y, int id);
int  sb_raycast(struct sbp_t* sbox, 
        int start_x, int start_y,
        int end_x,  int end_y,
        int* hit_x, int* hit_y);



// ---- DRAWING FUNCTIONS ----

void sb_setpixel(struct sbp_t* sbox, 
        float x, float y,
        float r, float g, float b);

void sb_fillcircle(struct sbp_t* sbox, 
        float fx, float fy, float radius,
        float r, float g, float b);

void sb_setline(struct sbp_t* sbox, 
        int x0, int y0, int x1, int y1,
        float r, float g, float b);

void sb_setbox(struct sbp_t* sbox,
        float x, float y, float w, float h,
        float r, float g, float b);

void sb_settex(struct sbp_t* sbox, 
        float x, float y,
        struct texture_t* tex);




