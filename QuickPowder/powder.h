/*
	powder.h ~ RL
*/

#pragma once

#define POWDER_TICK_RATE	(1 / 20.0)

typedef enum powder_type
{
	TYPE_AIR,
	TYPE_GROUND,
	TYPE_SAND,
} powder_type_t;

void powder_update(double delta);
void powder_render(double delta);
void powder_key_clicked(char key);
void powder_mouse_down(int x, int y);
void powder_query_at(int x, int y);