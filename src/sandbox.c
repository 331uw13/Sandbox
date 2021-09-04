#include <stdio.h>

#include "sandbox.h"
#include "shader.h"

static int program_color_uniform;



void cursor_pos_callback(GLFWwindow* win, double x, double y) {
	SANDBOX sb = (SANDBOX)glfwGetWindowUserPointer(win);
	sb->cursor.x = x;
	sb->cursor.y = y;
}

void key_callback(GLFWwindow* win, int key, int sc, int action, int mods) {
	SANDBOX sb = (SANDBOX)glfwGetWindowUserPointer(win);

	if(action == GLFW_PRESS) {
		switch(key) {

			case GLFW_KEY_ESCAPE:
				sb->flags |= F_CLOSE;
				break;


		
			default: break;
		}
	}

}



SANDBOX create_sandbox() {
	SANDBOX sb = NULL;
	if((sb = malloc(sizeof *sb))) {
		sb->world_size = (PX_W*PX_H)*sizeof *sb->world;
		sb->world = malloc(sb->world_size);
		sb->flags = 0;
		sb->win = NULL;
		sb->win_size.x = PX_W*PX_SIZE;
		sb->win_size.y = PX_H*PX_SIZE;
		sb->time = 0.0;

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
		free(sb->world);
		free(sb);
	}
}


int init_sandbox(SANDBOX sb) {
	int res = 0;

	if(sb != NULL) {

		// Initialize GLFW

		if(!glfwInit()) {
			fprintf(stderr, "Failed to initialize GLFW!\n");
			sb->flags |= F_CLOSE;
			goto finish;
		}
		
		sb->flags |= F_INIT_GLFW;
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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
		glfwSetInputMode(sb->win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glfwSwapInterval(1);
		glfwSetWindowUserPointer(sb->win, sb);

		glfwSetCursorPosCallback(sb->win, cursor_pos_callback);
		glfwSetKeyCallback(sb->win, key_callback);


		// Initialize GLEW

		if(glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW!\n");
			sb->flags |= F_CLOSE;
			goto finish;
		}

		sb->flags |= F_INIT_GLEW;
		res = 1;


		// Create shaders

		int shaders[] = {
			compile_shader(VERTEX_SHADER_SRC, GL_VERTEX_SHADER),
			compile_shader(FRAGMENT_SHADER_SRC, GL_FRAGMENT_SHADER)
		};

		if((sb->program = create_program(shaders, sizeof shaders)) > 0) {
			program_color_uniform = glGetUniformLocation(sb->program, "color");
		}
		
		glDeleteProgram(shaders[0]);
		glDeleteProgram(shaders[1]);

		glPointSize(PX_SIZE);
	}


finish:
	return res;
}

void update_frame(SANDBOX sb) {
	glfwPollEvents();
	glfwSwapBuffers(sb->win);
}

void draw_pixel(SANDBOX sb, int col, int row, u8 on_grid) {
	glBegin(GL_POINTS);
	if(on_grid) {
		col *= PX_SIZE;
		row *= PX_SIZE;
	}
	glVertex2f(((float)col/((float)sb->win_size.x/2))-1.0, 1.0-(float)row/((float)sb->win_size.y/2));
	glEnd();
}

void use_color(float r, float g, float b) {
	glUniform3f(program_color_uniform, r/COLOR_DETAIL, g/COLOR_DETAIL, b/COLOR_DETAIL);
}

void draw_line(SANDBOX sb, u16 x0, u16 y0, u16 x1, u16 y1) {
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
		draw_pixel(sb, x0, y0, 1);
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





