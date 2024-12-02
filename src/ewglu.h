#ifndef EEIUWIES_GLUTIL_H
#define EEIUWIES_GLUTIL_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdint.h>


// TODO: 
// textures
// vertex buffer object stuff
// vertex array object stuff
// more useful things.


int ewglu_init_glfw();
// * to set window hints 
// https://www.glfw.org/docs/latest/window_guide.html#window_hints_wnd
// set them with glfwWindowHint() after calling ewglu_init_glfw().
GLFWwindow* ewglu_init(
        int window_width,
        int window_height,
        const char* window_title,
        GLFWmonitor* monitor,
        uint32_t gl_version_major,
        uint32_t gl_version_minor);


// ---- SHADERS ----
// TODO: #include "other_shader.glsl"
//
uint32_t ewglu_compile_shader_file(const char* filepath, GLenum shader_type);
uint32_t ewglu_compile_shader  (const char* source, GLenum shader_type);
int      ewglu_link_program    (uint32_t program, uint32_t* shaders, uint64_t num_shaders);
uint32_t ewglu_create_program  (const char* vertex_src, const char* fragment_src);
void     ewglu_delete_program  (uint32_t program);
int      ewglu_get_uniformloc  (uint32_t program, const char* name);


// ---- TEXTURES ----

// for now just creates very simple texture
uint32_t ewglu_create_texture2D(GLenum type, GLenum format, int width, int height);
void ewglu_delete_texture(uint32_t tex);


float normalize (float t, float min, float max);
float lerp      (float t, float min, float max);
float map       (float t, float src_min, float src_max, float dst_min, float dst_max);
int   randomgen (int* seed);
int   randomi   (int* seed, int min, int max);
float randomf   (int* seed, float min, float max);
#define EWGLU_RANDOMGEN_MAX 0x7FFF

#endif // EEIUWIES_GLUTIL_H
