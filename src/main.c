#include "sandbox_engine.h"



int main() {
	SANDBOX sb = create_sandbox(150, 100, 5);
	if(sb == NULL) {
		return -1;
	}

	if(init_libs(sb)) {
		sandbox_ready(sb);
		sandbox_execute(sb);
	}

	destroy_sandbox(sb);
	return 0;
}








