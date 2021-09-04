#ifndef SANDBOX_ENGINE_H
#define SANDBOX_ENGINE_H

#include "sandbox.h"

void sandbox_ready(SANDBOX sb);
void sandbox_execute(SANDBOX sb);
void sandbox_update(SANDBOX sb);
int swap_material(SANDBOX sb, int from_x, int from_y, int to_x, int to_y);



#endif
