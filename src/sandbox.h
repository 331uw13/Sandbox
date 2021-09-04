#ifndef SANDBOX_H
#define SANDBOX_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "typedefs.h"
#include "types.h"

#define COLOR_DETAIL 16.0


#define F_CLOSE (1<<0)
#define F_INIT_GLFW (1<<1)
#define F_INIT_GLEW (1<<2)

#define PX_SIZE 8
#define PX_W 160
#define PX_H 110


#define GLSL_VERSION "#version 330\n"

#define VERTEX_SHADER_SRC \
GLSL_VERSION\
"layout(location=0) in vec2 p;\n"\
"void main(){"\
"gl_Position=vec4(p.xy,0.0,1.0);"\
"}"

#define FRAGMENT_SHADER_SRC \
GLSL_VERSION\
"uniform vec3 color;\n"\
"out vec4 out_color;\n"\
"void main(){"\
"out_color=vec4(color, 1.0);"\
"}"

#define SANDBOX struct sandbox_t*



struct sandbox_t {
	GLFWwindow* win;
	int flags;
	
	u16* world;
	u32 world_size;
	u32 cursor_index;

	float time;
	
	struct v2_t cursor;
	struct v2_t win_size;
	u32 program;
	u32 program_color_uniform;
	
};


SANDBOX create_sandbox();
void    destroy_sandbox(SANDBOX sb);

int init_sandbox(SANDBOX sb);
void update_frame(SANDBOX sb);
void use_color(float r, float g, float b);
void draw_line(SANDBOX sb, u16 x0, u16 y0, u16 x1, u16 y1);
void draw_pixel(SANDBOX sb, int col, int row, u8 on_grid);


#endif
