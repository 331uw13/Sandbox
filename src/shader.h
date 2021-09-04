#ifndef SHADER_H
#define SHADER_H

#include <stdio.h>
#include <stdlib.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>


int compile_shader(const char* src, int type);
int create_program(int*  shaders, int count);


#endif
