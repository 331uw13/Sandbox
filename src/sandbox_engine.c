#include "sandbox_engine.h"


void sandbox_ready(SANDBOX sb) {
	glClearColor(0.1, 0.1, 0.1, 1.0);
}



void sandbox_execute(SANDBOX sb) {
	while(!glfwWindowShouldClose(sb->win) && !(sb->flags & F_CLOSE)) {




		
		update_frame(sb);
	}
}



