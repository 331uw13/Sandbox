#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "ewglu.h"




static void glfw_error_callback(int err, const char* desc) {
    fprintf(stderr, "GLFW_ERROR(%i): %s\n", err, desc);
}


int ewglu_init_glfw() {
    glfwSetErrorCallback(glfw_error_callback);
    return glfwInit();
}


GLFWwindow* ewglu_init(
        int window_width,
        int window_height,
        const char* window_title,
        GLFWmonitor* monitor,
        uint32_t gl_version_major,
        uint32_t gl_version_minor)
{
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
    
    GLFWwindow* win = NULL;
    win = glfwCreateWindow(
            window_width, window_height,
            window_title, monitor, NULL);

    if(!win) {
        fprintf(stderr, "failed to create window.\n");
        glfwTerminate();
        goto error;
    }

    glfwMakeContextCurrent(win);

    GLenum glew_init_res = glewInit();
    if(glew_init_res != GLEW_OK) {
        fprintf(stderr, "ERROR: %s\n",
                glewGetErrorString(glew_init_res));
        glfwDestroyWindow(win);
        glfwTerminate();
        win = NULL;
        goto error;
    }


error:
    return win;
}

uint32_t ewglu_compile_shader_file(const char* filepath, GLenum shader_type) {
    uint32_t shader = 0;


    errno = 0;

    FILE* f = NULL;
    f = fopen(filepath, "r");

    if(!f) {
        fprintf(stderr, "(ERROR) %s: '%s' %s\n",
                __func__, filepath, strerror(errno));
        goto error;
    }

    uint64_t size = 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);

    fseek(f, 0, SEEK_SET);


    char* buf = NULL;
    buf = malloc(size);

    if(!buf) {
        fprintf(stderr, "(ERROR) %s: failed to allocate memory for file content buffer\n",
                __func__);
        goto error_and_close;
    }

    for(uint64_t i = 0; i < size; i++) {
        buf[i] = fgetc(f);
    }

    shader = ewglu_compile_shader(buf, shader_type);

    if(buf) {
        free(buf);
    }

error_and_close:
    fclose(f);

error:
    return shader;
}

uint32_t ewglu_compile_shader(const char* source, GLenum shader_type) {
    uint32_t shader = 0;

    if(!source) {
        fprintf(stderr, "(ERROR) '%s': source cant be NULL.\n",
                __func__);
        goto error;
    }

    shader = glCreateShader(shader_type);
    if(!shader) {
        fprintf(stderr, "(ERROR) '%s': failed to create shader\n",
                __func__);
        goto error;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    char* info_log = NULL;
    int info_log_size = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_size);
    if(info_log_size > 1) {
        info_log = malloc(info_log_size);
        if(!info_log) {
            fprintf(stderr, 
                    "(ERROR) failed to allocate memory for shader info log.\n");
            glDeleteShader(shader);
            shader = 0;
            goto error;
        }

        glGetShaderInfoLog(shader, info_log_size, NULL, info_log);
        fprintf(stderr, "\033[91m\033[7m Shader compile error! "
                "\033[0m\033[91m\n%s\033[0m",
                info_log);

        free(info_log);
        info_log = NULL;

        glDeleteShader(shader);
        shader = 0;
        goto error;
    }

error:
    return shader;
}

int ewglu_link_program(uint32_t program, uint32_t* shaders, uint64_t num_shaders) {
    int res = 0;

    if(!shaders || (num_shaders == 0) || (program == 0)) {
        goto error;
    }

    for(uint64_t i = 0; i < num_shaders; i++) {
        glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

    char* info_log = NULL;
    int info_log_size = 0;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_size);
    if(info_log_size > 1) {
        info_log = malloc(info_log_size);
        glGetProgramInfoLog(program, info_log_size, NULL, info_log);

        printf("\033[91m%s\033[0m\n", info_log);

        free(info_log);
        info_log = NULL;
    }
    else {
        res = 1;
    }

error:
    return res;
}

uint32_t ewglu_create_program(
        const char* vertex_src,
        const char* fragment_src) 
{
    uint32_t prog = 0;
    uint32_t vert_shader = 0;
    uint32_t frag_shader = 0;

    vert_shader = ewglu_compile_shader(vertex_src, GL_VERTEX_SHADER);
    if(!vert_shader) {
        fprintf(stderr, "(ERROR) vertex shader failed to compile\n");
        goto error;
    }

    frag_shader = ewglu_compile_shader(fragment_src, GL_FRAGMENT_SHADER);
    if(!frag_shader) {
        fprintf(stderr, "(ERROR) fragment shader failed to compile\n");
        goto error;
    }

    prog = glCreateProgram();
    if(!prog) {
        fprintf(stderr, "(ERROR) failed to create shader program\n");
        goto error;
    }


    uint32_t shaders[] = { vert_shader, frag_shader };
    ewglu_link_program(prog, shaders, 2);

error:
    
    if(vert_shader > 0) {
        glDeleteShader(vert_shader);
    }
    if(frag_shader > 0) {
        glDeleteShader(frag_shader);
    }
    
    return prog;
}

void ewglu_delete_program(uint32_t program) {
    if(program > 0) {
        glDeleteProgram(program);
    }
}

int ewglu_get_uniformloc(uint32_t program, const char* name) {
    int res = glGetUniformLocation(program, name);
    
    if(res == -1) {
        fprintf(stderr, "(ERROR) %s: Failed to get uniform location. name: '%s'\n",
                __func__, name);
    }

    return res;
}

uint32_t ewglu_create_texture2D(GLenum type, GLenum format, int width, int height) {
    uint32_t tex = 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return tex;
}

void ewglu_delete_texture(uint32_t tex) {
    if(tex > 0) {
        glDeleteTextures(1, &tex);
    }
}


float normalize(float t, float min, float max) {
    return (t - min) / (max - min);
}

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

int randomgen (int* seed) {
    *seed = 0x343FD * *seed + 0x269EC3;
    return (*seed >> 16) & EWGLU_RANDOMGEN_MAX;
}

int randomi(int* seed, int min, int max) {
    return randomgen(seed) % (max - min) + min;
}

float randomf(int* seed, float min, float max) {
    return ((float)randomgen(seed) / ((float)EWGLU_RANDOMGEN_MAX / (max - min))) + min;
}


