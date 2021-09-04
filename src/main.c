#include "sandbox_engine.h"

int main() {
	SANDBOX sb = create_sandbox();
	if(sb == NULL) {
		return -1;
	}

	if(init_sandbox(sb)) {
		sandbox_ready(sb);
		sandbox_execute(sb);
	}

	destroy_sandbox(sb);
	return 0;
}








