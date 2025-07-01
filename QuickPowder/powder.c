/*
	powder.c ~ RL
*/

#include "powder.h"
#include "screen.h"

static powder_type_t canvas[SCREEN_WIDTH * SCREEN_HEIGHT];

static inline powder_type_t powder_get(int x, int y)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return TYPE_GROUND;
	}
	return canvas[x + y * SCREEN_WIDTH];
}

static inline void powder_set(int x, int y, powder_type_t type)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return;
	}
	canvas[x + y * SCREEN_WIDTH] = type;
}

void powder_update(double delta)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 1; y >= 0; y--)
		{
			powder_type_t curr = powder_get(x, y);
			switch (curr)
			{
			case TYPE_SAND:
				if (powder_get(x, y + 1) == TYPE_AIR)
				{
					powder_set(x, y, TYPE_AIR);
					powder_set(x, y + 1, TYPE_SAND);
				}
				break;
			}
		}
	}
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