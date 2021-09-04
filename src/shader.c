#include "shader.h"



int compile_shader(const char* src, int type) {
	int shader = 0;

	if(src != NULL) {
		shader = glCreateShader(type);
		if(shader <= 0) {
			fprintf(stderr, "(ERROR): Failed to create shader!\n");
			goto finish;
		}

		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);

		int p = 0;
		int log_length = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &p);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		
		if(!p && log_length > 0) {
			char* msg = malloc(log_length);
			if(msg != NULL) {
				glGetShaderInfoLog(shader, log_length, NULL, msg);
				fprintf(stderr, "=====[ SHADER_INFO_LOG ]=========\n%s", msg);
				free(msg);
			}
		}

	}

finish:
	return shader;
}


int create_program(int* shaders, int count) {
	int program = 0;

	if(shaders != NULL && count > 0) {
		program = glCreateProgram();
		if(program <= 0) {
			fprintf(stderr, "(ERROR): Failed to create program!\n");
			goto finish;
		}

		for(int i = 0; i < count; i++) {
			glAttachShader(program, shaders[i]);
		}

		glLinkProgram(program);

		int p = 0;
		int log_length = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &p);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		
		if(!p && log_length > 0) {
			char* msg = malloc(log_length);
			if(msg != NULL) {
				glGetProgramInfoLog(program, log_length, NULL, msg);
				fprintf(stderr, "=====[ PROGRAM_INFO_LOG ]=========\n%s", msg);
				free(msg);
			}
		}
	}

finish:
	return program;
}

