/*
	powder.c ~ RL
*/

#include "powder.h"
#include "screen.h"

static powder_type_t canvas[SCREEN_WIDTH * SCREEN_HEIGHT];

static inline powder_type_t powder_get(int x, int y)
{
	return canvas[x + y * SCREEN_WIDTH];
}

static inline void powder_set(int x, int y, powder_type_t type)
{
	canvas[x + y * SCREEN_WIDTH] = type;
}

void powder_update(double delta)
{

}

void powder_render(double delta)
{
	screen_set_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BLACK);
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			switch (powder_get(x, y))
			{
			case TYPE_SAND:
				screen_set_pixel(x, y, COLOR_LIGHT_YELLOW);
				break;
			}
		}
	}
}

void powder_key_clicked(char key)
{

}

void powder_mouse_down(int x, int y)
{
	powder_set(x, y, TYPE_SAND);
}