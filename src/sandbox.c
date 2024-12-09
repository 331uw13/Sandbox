#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "sandbox.h"

void run_sandbox(struct sbp_t* sbox,
        void(*loop_callback)(struct sbp_t* sbox, void*), void* userptr) {
    


    while(!glfwWindowShouldClose(sbox->win) && sbox->running) {
        const double t_framestart = glfwGetTime();
        
        glClear(GL_COLOR_BUFFER_BIT);

        if(!(sbox->flags & F_NOMOUSEPOS)) {
            glfwGetCursorPos(sbox->win, &sbox->mouse_col, &sbox->mouse_row);
            if(sbox->mouse_col > 0.0) {
                sbox->mouse_col /= PIXELSIZE;
            }
            if(sbox->mouse_row > 0.0) {
                sbox->mouse_row /= PIXELSIZE;
            }
        }

        sbox->time = glfwGetTime();


        // map the buffer for pixels
        sbox->pixels.buffer = glMapNamedBuffer(sbox->pixels.vbo, GL_WRITE_ONLY);
        if(!sbox->pixels.buffer) {
            fprintf(stderr, "(ERROR) %s: Failed to map buffer.\n",
                    __func__);
            break;
        }

        // clear previous stuff drawn previous frame.
        if(sbox->flags & F_CLEARSCREEN) {

            glClearTexImage(sbox->effect.tex, 0, GL_RGB, GL_FLOAT, 0);
            
            // set everything in buffer to -2 to be offscreen
            // 0 will be center of the screen
            // -1 will be the corner.
            // this is because of opengl coordinate system.
            memset(sbox->pixels.buffer, -2, sbox->pixels.buffer_sizeb);
        }

        // pixels to buffer and effects are updated now by user.
        loop_callback(sbox, userptr);


        // first unmap the buffer and draw pixels first 
        // then draw the texture that contains all different effects on top.
        // TODO: flag 'F_NOEFFECTS' to not render and not clear the texture.


        glUnmapNamedBuffer(sbox->pixels.vbo);
        sbox->pixels.buffer = NULL;


        // pixels
        {
            glUseProgram(sbox->pixels.shader);
            glBindVertexArray(sbox->pixels.vao);
            glDrawArrays(GL_POINTS, 0, sbox->pixels.num_maxpixels);

        }


        // effects
        {
            glUseProgram(sbox->effect.shader);
            glUniform1f(sbox->effect.unilocs[EFFECT_UNILOC_ITIME], sbox->time);
            
            glBindTexture(sbox->effect.tex, GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            
            glBindVertexArray(sbox->effect.vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }


        sbox->mouse_scroll = 0;
        sbox->mouse_left_down = 0;
        sbox->mouse_right_down = 0;
        sbox->mouse_middle_down = 0;


        glfwSwapBuffers(sbox->win);
        
        sbox->dt = glfwGetTime() - t_framestart;

        if(sbox->render_mode == RENDERMODE_POLL_EVENTS) {
            glfwPollEvents();
        }
        else {
            glfwWaitEvents();
        }
    }

}

void free_sandbox(struct sbp_t* sbox) {
    glDeleteVertexArrays(1, &sbox->effect.vao);
    glDeleteBuffers(1, &sbox->effect.vbo);
    glDeleteBuffers(1, &sbox->effect.ebo);
    glDeleteBuffers(1, &sbox->effect.ubo);

    glDeleteVertexArrays(1, &sbox->pixels.vao);
    glDeleteBuffers(1, &sbox->pixels.vbo);

    ewglu_delete_program(sbox->pixels.shader);
    ewglu_delete_program(sbox->effect.shader);
    ewglu_delete_texture(sbox->effect.tex);
    
    sb_free_rcbuf(sbox);

    if(sbox->win) {
        glfwDestroyWindow(sbox->win);
    }

    glfwTerminate();
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    struct sbp_t* sbox = (struct sbp_t*) glfwGetWindowUserPointer(window);
    if(sbox) {
        sbox->mouse_scroll = (int)yoffset;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    struct sbp_t* sbox = (struct sbp_t*) glfwGetWindowUserPointer(window);
    if(sbox && (action == GLFW_PRESS)) {
        switch(button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                sbox->mouse_left_down = 1;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                sbox->mouse_right_down = 1;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                sbox->mouse_middle_down = 1;
                break;
        }
    }
}

static int _compile_shaderprog(struct sbp_t* sbox, 
        const char* name_to_stdout,
        const char* vert_src, const char* frag_src) 
{
    printf(" \033[90m,--\033[0m %s.\n", name_to_stdout);
    int result = ewglu_create_program(vert_src, frag_src);
    if(result) {
        printf(" \033[90m`-> \033[32mok!\033[0m\n");
    }
    else {
        printf(" \033[90m`-> \033[31mfailed!\033[0m\n");
    }

    return result;
}

int init_sandbox(struct sbp_t* sbox, int width, int height, const char* window_name) {
    int result = 0;
    //struct sbp_t* sbox = NULL;
    //sbox = malloc(sizeof *sbox);


    sbox->running = 0;
    sbox->time = 0.0;
    sbox->dt = 0.0;
    sbox->mouse_col = 0.0;
    sbox->mouse_row = 0.0;
    sbox->mouse_scroll = 0;
    sbox->flags = 0;
    sbox->render_mode = RENDERMODE_POLL_EVENTS;

    if(!ewglu_init_glfw()) {
        goto error;
    }

    glfwWindowHint(GLFW_RESIZABLE, 0);
    sbox->win = NULL;
    sbox->win = ewglu_init(width, height, window_name, NULL, 4, 3);
    if(!sbox->win) {
        goto error;
    }

    printf("glew version: %s\n", glewGetString(GLEW_VERSION));
    printf("opengl version: %s\n", glGetString(GL_VERSION));

    glfwSetWindowUserPointer(sbox->win, sbox);
    glfwSetScrollCallback(sbox->win, scroll_callback);
    glfwSetMouseButtonCallback(sbox->win, mouse_button_callback);

    glfwGetWindowSize(sbox->win, &sbox->win_width, &sbox->win_height);
    
    if(sbox->win_width * sbox->win_height <= 0) {
        fprintf(stderr, "Window size seems to be way too small\n");
        goto error;
    }

    glPointSize(PIXELSIZE);
    glfwSwapInterval(1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
   
    sbox->effect = (struct effect_t) {
        .vao = 0,
        .vbo = 0,
        .ebo = 0,
        .ubo = 0,
        .tex = 0,
        .shader = 0,
        .unilocs = { 0 }
    };

    sbox->pixels = (struct pixels_t) {
        .vao = 0,
        .vbo = 0,
        .buffer = NULL,
        .buffer_sizeb = 0,
        .num_maxpixels = 0
    };


    sbox->max_col = sbox->win_width / PIXELSIZE;
    sbox->max_row = sbox->win_height / PIXELSIZE;
    sbox->center_col = sbox->max_col / 2;
    sbox->center_row = sbox->max_row / 2;
    sbox->pixels.num_maxpixels = sbox->max_col * sbox->max_row;

    sbox->num_lights = 0;
    sbox->mouse_left_down = 0;

        
    // initialize effect structure.
    // it has one rectangle and
    // this it will have all overlay effects (lights, fog, etc..)
    {
        const float vertices[] = {
            // position     // textures coordinates
             1.0,  1.0,     1.0, 1.0,
             1.0, -1.0,     1.0, 0.0,
            -1.0, -1.0,     0.0, 0.0,
            -1.0,  1.0,     0.0, 1.0
        };

        const unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };


        glGenVertexArrays(1, &sbox->effect.vao);
        glBindVertexArray(sbox->effect.vao);
        
        glGenBuffers(1, &sbox->effect.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, sbox->effect.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        
        glGenBuffers(1, &sbox->effect.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sbox->effect.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        const size_t stridesize = sizeof(float) * 4;

        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stridesize, (void*)0);
        glEnableVertexAttribArray(0);

        // texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stridesize, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        sbox->effect.shader = _compile_shaderprog
            (sbox, "Effect shader", SANDBOX_EFFECT_VERTEX_SRC, SANDBOX_EFFECT_FRAGMENT_SRC);
        if(!sbox->effect.shader) {
            goto error;
        }


        sbox->effect.unilocs[EFFECT_UNILOC_INUMLIGHTS] = 
            ewglu_get_uniformloc(sbox->effect.shader, "f_num_lights");
        
        sbox->effect.unilocs[EFFECT_UNILOC_ITIME] = 
            ewglu_get_uniformloc(sbox->effect.shader, "f_time");


        // create uniform buffer object for lights.
        // and bind it,
        // if you want to create more shaders that uses this block
        // you have to bind it to the shader.

        glGenBuffers(1, &sbox->effect.ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, sbox->effect.ubo);
        
        // allocate memory for it.
        const size_t ubomemsize = EFFECTSHADER_LIGHT_T_SIZEB * MAX_LIGHTS;
        glBufferData(GL_UNIFORM_BUFFER, ubomemsize, NULL, GL_STATIC_DRAW); 

        unsigned int lights_ubindex = glGetUniformBlockIndex(sbox->effect.shader, "lights_uniform_block");
        if(lights_ubindex == GL_INVALID_INDEX) {
            fprintf(stderr, "(WARNING) %s: Failed to get uniform block index for lights\n",
                    __func__);
        }

        glUniformBlockBinding(sbox->effect.shader, lights_ubindex,
                EFFECTSHADER_LIGHTS_BINDING_POINT);

        glBindBufferRange(
                GL_UNIFORM_BUFFER,
                EFFECTSHADER_LIGHTS_BINDING_POINT,
                sbox->effect.ubo,
                0, ubomemsize);

        sbox->effect.tex = ewglu_create_texture2D(GL_FLOAT, GL_RGB, sbox->max_col, sbox->max_row);
    }


    // initialize pixel structure.
    // allocating enough memory to hold max_col * max_row number of pixels
    // and then mapping the buffer before calling 'update_callback' in main loop.
    // user can then update pixel positions very efficiently
    // the buffer is unmapped before drawing everything at once.
    {
        const size_t stridesize = sizeof(float) * 5;
       
        sbox->pixels.buffer_sizeb = stridesize * sbox->pixels.num_maxpixels;
        glGenVertexArrays(1, &sbox->pixels.vao);
        glBindVertexArray(sbox->pixels.vao);    

        glGenBuffers(1, &sbox->pixels.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, sbox->pixels.vbo);
        glBufferData(GL_ARRAY_BUFFER, sbox->pixels.buffer_sizeb, NULL, GL_STREAM_DRAW);

        // pixel position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stridesize, (void*)0);
        glEnableVertexAttribArray(0);

        // colors
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stridesize, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);


        sbox->pixels.shader = _compile_shaderprog
            (sbox, "Pixel shader", SANDBOX_PIXEL_VERTEX_SRC, SANDBOX_PIXEL_FRAGMENT_SRC);
    
        if(!sbox->pixels.shader) {
            goto error;
        }

    }


    sbox->rcbuf = NULL;
    sbox->rcbuf_avail = 0,

    sbox->running = 1;
    sbox->flags |= F_CLEARSCREEN;



    for(size_t i = 0; i < MAX_LIGHTS; i++) {
        struct light_t* l = &sbox->lights[i];
        l->x = 0.0;
        l->y = 0.0;
        l->r = 0.0;
        l->g = 0.0;
        l->b = 0.0;
        l->strength = 0.0;
        l->index = 0;
    }


    printf("\033[34m+ Ready.\033[0m\n");

    result = 1;
error:
    return result;
}


// UTILITY FUNCTIONS

void sb_rainbow_palette(float t, float* r, float* g, float* b) {
    // for more information about this check out:
    // https://iquilezles.org/articles/palettes/
    *r = 0.5+0.5 * cos(_2PI * t);
    *g = 0.5+0.5 * cos(_2PI * (t+0.33));
    *b = 0.5+0.5 * cos(_2PI * (t+0.67));
}

float vdistance(float x0, float y0, float x1, float y1) {
    float dx = x0 - x1;
    float dy = y0 - y1;
    return sqrt(dx*dx + dy*dy);
}

float vdot(float x0, float y0,  float x1, float y1) {
    return (x0 * x1) + (y0 * y1);
}

float vlength(float x, float y) {
    return sqrt(vdot(x, y, x, y));
}

float vangle(float x0, float y0, float x1, float y1) {
    float delx = x1 - x0;
    float dely = y1 - y0;
    return (atan2(dely, delx)*180.0)/_PI;
}

size_t sb_getindexp(struct sbp_t* sbox, int x, int y) {
    if(x < 0) { x = 0; }
    if(y < 0) { y = 0; }

    size_t index = y * sbox->max_col + x;

    if(index >= sbox->pixels.num_maxpixels) {
        index = sbox->pixels.num_maxpixels-1;
    }


    return index;
}

void sb_show_cursor(struct sbp_t* sbox, int mode) {
    glfwSetInputMode(sbox->win, GLFW_CURSOR, (mode) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}


// RAYCAST FUNCTIONS

int sb_init_rcbuf(struct sbp_t* sbox) {
    int res = 0;

    if(sbox->rcbuf != NULL) {
        fprintf(stderr, "raycast buffer is already initialized ???\n");
        goto error;
    }

    size_t rcbufsize = sbox->pixels.num_maxpixels * sizeof *sbox->rcbuf;
    sbox->rcbuf = malloc(rcbufsize);

    if(!sbox->rcbuf) {
        fprintf(stderr, "%s | failed to allocate memory for raycast buffer %li bytes of memory\n",
                __func__, rcbufsize);
        goto error;
    }

    memset(sbox->rcbuf, 0, rcbufsize);

    sbox->rcbuf_avail = 1;
    res = 1;

error:
    return res;
}

void sb_free_rcbuf(struct sbp_t* sbox) {
    if(sbox->rcbuf) {
        free(sbox->rcbuf);
    }
    sbox->rcbuf_avail = 0;
}

void sb_rcbuf_setid(struct sbp_t* sbox, int x, int y, int id) {
    if(sbox->rcbuf && sbox->rcbuf_avail) {
        sbox->rcbuf[sb_getindexp(sbox, x, y)] = id;
    }
}

int sb_raycast(struct sbp_t* sbox, 
        int start_x, int start_y,
        int end_x,  int end_y,
        int* hit_x, int* hit_y)
{
    int id = 0;

    int width = end_x - start_x;
    int height = end_y - start_y;
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

        if((id = sbox->rcbuf[sb_getindexp(sbox, start_x, start_y)])
                != RAYCAST_AIRID) {

            if(hit_x) {
                *hit_x = start_x;
            }
            if(hit_y) {
                *hit_y = start_y;
            }

            break;
        }

        numerator += shortest;
        if(numerator > longest) {
            numerator -= longest;
            start_x += dx0;
            start_y += dy0;
        }
        else {
            start_x += dx1;
            start_y += dy1;
        }
    }

    return id;
}


// "DRAWING" FUNCTIONS

void setpixel(struct sbp_t* sbox, float x, float y,
        float r, float g, float b)
{
    x = floorf(x);
    y = floorf(y);

    if(x * y < 0) { // discard any negative coordinate
        return;
    }

    if(x >= sbox->max_col) {
        return;
    }
    if(y >= sbox->max_row) {
        return;
    }


    size_t index = (int)y * sbox->max_col + (int)x;

    if((index < sbox->pixels.num_maxpixels) && sbox->pixels.buffer) {
        index *= 5;
        sbox->pixels.buffer[index]   = map(x, 0.0, sbox->max_col, -1.0,  1.0);
        sbox->pixels.buffer[index+1] = map(y, 0.0, sbox->max_row,  1.0, -1.0);
        sbox->pixels.buffer[index+2] = r;
        sbox->pixels.buffer[index+3] = g;
        sbox->pixels.buffer[index+4] = b;
    }
}

void fillcircle(struct sbp_t* sbox, 
        float fx, float fy, float radius,
        float r, float g, float b)
{

    if(radius == 1.0) {
        setpixel(sbox, fx, fy, r, g, b);
        return;
    }

    int ystart = fy - radius;
    int yend   = fy + radius;
    
    int xstart = fx - radius;
    int xend   = fx + radius;
    

    for(int y = ystart; y <= yend; y++) {
        for(int x = xstart; x <= xend; x++) {
   
            //printf("%.3f, %.3f\n", x, y);

            float dst = vdistance(x+0.5, y+0.5, fx, fy);

            if(dst <= radius) {
                setpixel(sbox, x, y,  r, g, b);
            }
        }
    }
}

void setline(struct sbp_t* sbox, 
        int x0, int y0, int x1, int y1,
        float r, float g, float b)
{
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
        
        setpixel(sbox, x0, y0, r,g,b);

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

void setbox(struct sbp_t* sbox,
        float x, float y, float w, float h,
        float r, float g, float b)
{
    const int width = (int)floorf(w);
    const int height = (int)floorf(h);
    const int ix = (int)floorf(x);
    const int iy = (int)floorf(y);

    for(int by = 0; by < height; by++) {
        for(int bx = 0; bx < width; bx++) {
            setpixel(sbox, ix+bx, iy+by, r, g, b);
        }
    }

}

void settex(struct sbp_t* sbox, 
        float x, float y,
        struct texture_t* tex)
{

    if(!tex) {
        return;
    }

    if(!tex->data) {
        return;
    }

    for(size_t i = 0; i < tex->num_pixels; i++) {
        struct texpx_t* tp = &tex->data[i];

        setpixel(sbox, x+tp->x, y+tp->y, tp->red, tp->grn, tp->blu);

    }


}
