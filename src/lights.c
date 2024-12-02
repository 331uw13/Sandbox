#include <stdio.h>

#include "lights.h"
#include "sandbox.h"


struct light_t* setup_light(
        struct sandbox_t* sbox,
        int req_index,
        float x, float y,
        float r,
        float g,
        float b,
        float strength,
        float radius,
        float effect
        )
{
    struct light_t* ptr = NULL;

    if(req_index < 0) {
        req_index = sbox->num_lights;
    }

    if(req_index >= MAX_LIGHTS) {
        fprintf(stderr, "(ERROR) %s: req_index is out of bounds.\n",
                __func__);
        goto error;
    }

    ptr = &sbox->lights[req_index];

    ptr->r = r;
    ptr->g = g;
    ptr->b = b;
    ptr->x = x;
    ptr->y = y;
    ptr->strength = strength;
    ptr->radius = radius;
    ptr->effect = effect;
    ptr->index = req_index;

    glUseProgram(sbox->effect.shader);
    sbox->num_lights++;


    glUniform1i(sbox->effect.unilocs[EFFECT_UNILOC_INUMLIGHTS], sbox->num_lights);

    update_light_allvars(sbox, ptr);

error:
    return ptr;
}


void update_light_allvars(struct sandbox_t* sbox, struct light_t* light) {
    update_light_pos(sbox, light);
    update_light_color(light);
    update_light_strength(light);
    update_light_radius(light);
    update_light_effect(light);
}


void update_light_pos(struct sandbox_t* sbox, struct light_t* light) {

    float pos[2] = {
        /* X */ map(light->x, 0.0, sbox->max_col, -1.0,  1.0),
        /* Y */ map(light->y, 0.0, sbox->max_row,  1.0, -1.0)
    };

    size_t offset = light->index * EFFECTSHADER_LIGHT_T_SIZEB;
    size_t size = sizeof(float) * 2;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pos);
}

void update_light_color(struct light_t* light) {
    size_t offset = light->index * EFFECTSHADER_LIGHT_T_SIZEB + 16;
    size_t size = sizeof(float) * 3;
    float color[3] = { light->r, light->g, light->b };
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, color);
}

void update_light_strength(struct light_t* light) {
    size_t offset = light->index * EFFECTSHADER_LIGHT_T_SIZEB + 32;
    size_t size = sizeof(float);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->strength);
}

void update_light_radius(struct light_t* light) {
    size_t offset = light->index * EFFECTSHADER_LIGHT_T_SIZEB + 32+4;
    size_t size = sizeof(float);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->radius);
}

void update_light_effect(struct light_t* light) {
    size_t offset = light->index * EFFECTSHADER_LIGHT_T_SIZEB + 32+4*2;
    size_t size = sizeof(float);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->effect);
}



