/*
	powder.h ~ RL
*/

#pragma once

#define POWDER_TICK_RATE	(1 / 20.0)

typedef enum powder_type
{
	TYPE_AIR		= 0x01,
	TYPE_GROUND		= 0x02,
	TYPE_SAND		= 0x04,
	TYPE_WATER		= 0x08,
} powder_type_t;

#define POWDER_IS_LIQUID(type)	((type) & TYPE_WATER)
#define POWDER_IS_GROUND(type)	((type) & TYPE_GROUND)
#define POWDER_IS_SOLID(type)	((type) & (TYPE_GROUND | TYPE_SAND))

void powder_init(unsigned int seed);
void powder_update(double delta);
void powder_render(double delta);
void powder_key_clicked(char key);
void powder_mouse_primary_down(int x, int y);
void powder_mouse_aux_down(int x, int y);