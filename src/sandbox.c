#include <stdio.h>

#include "sandbox.h"


SANDBOX create_sandbox(u16 w_px, u16 h_px, u16 px_size) {
	SANDBOX sb = NULL;
	if((sb = malloc(sizeof *sb))) {
		sb->flags = 0;
		sb->win = NULL;
		sb->pixels.x = w_px;
		sb->pixels.y = h_px;
		sb->px_size = px_size;
		sb->win_size.x = px_size*w_px;
		sb->win_size.y = px_size*h_px;
		sb->world = 0;
		sb->pbuffer = 0;
		sb->vbo = 0;


		float stuff[] = {
			-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f 
		};

		glGenBuffers(1, &sb->vbo);
		//glBindBuffer(GL_ARRAY_BUFFER, sb->vbo);
		//glBufferData(GL_ARRAY_BUFFER, sizeof stuff, stuff, GL_STATIC_DRAW);

		/*
		glGenTextures(1, &sb->world);
		glBindTexture(GL_TEXTURE_2D, sb->world);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sb->win_size.x, sb->win_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		*/

		//glGenBuffers(1, &sb->pbuffer);
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, sb->pbuffer);
		//glBufferData(GL_PIXEL_PACK_BUFFER, sb->win_size.x*sb->win_size.y, NULL, GL_STREAM_DRAW);
	}
	else {
		fprintf(stderr, "Failed to allocate %li bytes of memory! for 'SANDBOX'\n", sizeof *sb);
	}

	return sb;
}



void destroy_sandbox(SANDBOX sb) {
	if(sb != NULL) {
		if(sb->flags & F_INIT_GLFW) {
			if(sb->win != NULL) {
				glfwDestroyWindow(sb->win);
			}
			glfwTerminate();
		}
		if(sb->flags & F_INIT_GLEW) {
			glDeleteTextures(1, &sb->world);
		}
		free(sb);
	}
}


int init_libs(SANDBOX sb) {
	int res = 0;

	if(sb != NULL) {

		// Initialize GLFW

		if(!glfwInit()) {
			fprintf(stderr, "Failed to initialize GLFW!\n");
			sb->flags |= F_CLOSE;
			goto finish;
		}
		
		sb->flags |= F_INIT_GLFW;
		glfwWindowHint(GLFW_RESIZABLE, 0);
		glfwWindowHint(GLFW_CENTER_CURSOR, 1);
		glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

		if(!(sb->win = glfwCreateWindow(sb->win_size.x, sb->win_size.y, "sandbox", NULL, NULL))) {
			fprintf(stderr, "Failed to create window!\n");
			sb->flags |= F_CLOSE;
			goto finish;
		}

		glfwMakeContextCurrent(sb->win);
		

		// Initialize GLEW

		if(glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW!\n");
			sb->flags |= F_CLOSE;
			goto finish;
		}

		sb->flags |= F_INIT_GLEW;
		res = 1;
	}


finish:
	return res;
}

void update_frame(SANDBOX sb) {
	glfwSwapBuffers(sb->win);
	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT);
}



