/*
	powder.c ~ RL
*/

#include "powder.h"
#include "screen.h"
#include <stdbool.h>

static powder_type_t canvas[SCREEN_WIDTH * SCREEN_HEIGHT];
static powder_type_t current = TYPE_SAND;

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

static bool powder_is_grounded(int x, int y)
{
	powder_type_t query = powder_get(x, y);
	if (query == TYPE_AIR)
	{
		return false;
	}
	for (; powder_get(x, y) == query; y++);
	return powder_get(x, y) == TYPE_GROUND;
}

void powder_update(double delta)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 1; y >= 0; y--)
		{
			if (powder_is_grounded(x, y) && powder_get(x, y) == TYPE_SAND && powder_get(x, y - 1) == TYPE_SAND)
			{
				if (powder_get(x - 1, y) == TYPE_AIR)
				{
					powder_set(x, y - 1, TYPE_AIR);
					powder_set(x - 1, y, TYPE_SAND);
				}
				else if (powder_get(x + 1, y) == TYPE_AIR)
				{
					powder_set(x, y - 1, TYPE_AIR);
					powder_set(x + 1, y, TYPE_SAND);
				}
			}
		}
	}

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
			case TYPE_GROUND:
				screen_set_pixel(x, y, COLOR_DARK_GRAY);
				break;
			}
		}
	}
}

void powder_key_clicked(char key)
{
	if (key >= '0' && key <= '2')
	{
		key -= '0';
		current = key;
	}
}

void powder_mouse_down(int x, int y)
{
	if (powder_get(x, y) == TYPE_AIR)
	{
		powder_set(x, y, current);
	}
}

void powder_query_at(int x, int y)
{

}