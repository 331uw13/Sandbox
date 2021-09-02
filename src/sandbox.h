#ifndef SANDBOX_H
#define SANDBOX_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "typedefs.h"
#include "util.h"

// Flags.
#define F_CLOSE (1<<0)
#define F_INIT_GLFW (1<<1)
#define F_INIT_GLEW (1<<2)


#define SANDBOX struct sandbox_t*

struct v2_t {
	int x;
	int y;
};

struct sandbox_t {
	GLFWwindow* win;
	int flags;
	
	struct v2_t pixels;  // Amount of pixels horizontally and vertically.
	struct v2_t win_size;
	u16 px_size;
	u32 world;
	u32 pbuffer;
	u32 vbo;
};


SANDBOX create_sandbox(u16 w_px, u16 h_px, u16 px_size);
void    destroy_sandbox(SANDBOX sb);

int init_libs(SANDBOX sb);
void update_frame(SANDBOX sb);



#endif
