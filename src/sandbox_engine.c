#include <math.h>
#include <stdio.h>

#include "sandbox_engine.h"



void sandbox_ready(SANDBOX sb) {
	glClearColor(0.1, 0.1, 0.1, 1.0);
}


void sandbox_execute(SANDBOX sb) {
	while(!glfwWindowShouldClose(sb->win) && !(sb->flags & F_CLOSE)) {
		glClear(GL_COLOR_BUFFER_BIT);
		sb->time = glfwGetTime();
		glUseProgram(sb->program);


		int ix = sb->cursor.x/PX_SIZE;
		int iy = sb->cursor.y/PX_SIZE;
		sb->cursor_index = ix*PX_H+iy;

		if(glfwGetKey(sb->win, GLFW_KEY_D) == GLFW_PRESS) {
			sb->world[sb->cursor_index] = 1;
		}

		use_color(16, 16, 16);
		draw_pixel(sb, sb->cursor.x, sb->cursor.y, 0);

		sandbox_update(sb);

		use_color(9, 3, 8);
		for(int y = 0; y < PX_H; y++) {
			for(int x = 0; x < PX_W; x++) {
				if(sb->world[x*PX_H+y]) {
					draw_pixel(sb, x, y, 1);
				}
			}
		}

		update_frame(sb);
	}
}

#define SPOT(a, b) (sb->world[a*PX_H+b])


void sandbox_update(SANDBOX sb) {
	for(int y = PX_H; y > 0; y--) {
		for(int x = PX_W; x > 0; x--) {
			if(SPOT(x, y)) {
				if(!swap_material(sb, x, y, x, y+1)) {	
					if(!swap_material(sb, x, y, x+1, y+1)) {
						if(!swap_material(sb, x, y, x-1, y+1)) {
						}
					}
				}

			}
		}
	}
}

int swap_material(SANDBOX sb, int from_x, int from_y, int to_x, int to_y) {
	int res = 0;
	if((to_y >= 0 && to_y < PX_H-1) && (to_x >= 0 && to_x < PX_W-1)) {
		if(!SPOT(to_x, to_y)) {
			SPOT(from_x, from_y) = 0;
			SPOT(to_x, to_y) = 1;
			res = 1;
		}
	}
	return res;
}







