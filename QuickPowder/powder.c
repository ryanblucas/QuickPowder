/*
	powder.c ~ RL
*/

#include "powder.h"
#include "screen.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static powder_type_t canvas[SCREEN_WIDTH * SCREEN_HEIGHT];
static powder_type_t current = TYPE_SAND;

int powder_brush_size = 2;

void powder_init(unsigned int seed)
{
	srand(seed);
	for (int i = 0; i < sizeof canvas / sizeof * canvas; i++)
	{
		canvas[i] = TYPE_AIR;
	}
}

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

static bool powder_is_grounded(int x, int y, powder_type_t mask)
{
	powder_type_t query = powder_get(x, y);
	if (POWDER_IS_AIR(query))
	{
		return false;
	}
	for (; powder_get(x, y) == query; y++);
	return powder_get(x, y) & mask;
}

static void powder_update_spread(void)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 1; y >= 0; y--)
		{
			powder_type_t curr = powder_get(x, y), above = powder_get(x, y - 1);
			if (powder_is_grounded(x, y, TYPE_GROUND) && TYPE_SAND == curr && TYPE_SAND == above)
			{
				powder_type_t left = powder_get(x - 1, y),
					right = powder_get(x + 1, y);
				if (!POWDER_IS_SOLID(left))
				{
					powder_set(x, y - 1, TYPE_AIR);
					powder_set(x - 1, y, TYPE_SAND);
				}
				else if (!POWDER_IS_SOLID(right))
				{
					powder_set(x, y - 1, TYPE_AIR);
					powder_set(x + 1, y, TYPE_SAND);
				}
			}
			else if (powder_is_grounded(x, y, TYPE_GROUND | TYPE_SAND) && TYPE_WATER == curr && TYPE_WATER == above)
			{
				int lx, rx;
				for (lx = x; powder_get(lx, y) == TYPE_WATER; lx--);
				for (rx = x; powder_get(rx, y) == TYPE_WATER; rx++);
				int lwx = x - lx,
					rwx = rx - x;
				if (lwx > rwx && powder_get(rx, y) == TYPE_AIR)
				{
					powder_set(rx, y, TYPE_WATER);
					powder_set(x, y - 1, TYPE_AIR);
				}
				else if (lwx <= rwx && powder_get(lx, y) == TYPE_AIR)
				{
					powder_set(lx, y, TYPE_WATER);
					powder_set(x, y - 1, TYPE_AIR);
				}
			}
		}
	}
}

static void powder_update_gravity(void)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 1; y >= 0; y--)
		{
			powder_type_t curr = powder_get(x, y), down = powder_get(x, y + 1);
			switch (curr)
			{
			case TYPE_SAND:
			case TYPE_WATER:
				if (!POWDER_IS_SOLID(down))
				{
					powder_set(x, y, down);
					powder_set(x, y + 1, curr);
				}
				break;
			}
		}
	}
}

#define POWDER_LIQUID_UPDATE_FREQUENCY 0.25

static void powder_update_liquid(double delta)
{
	static double elapsed = 0.0;
	elapsed += delta;
	if (elapsed <= POWDER_LIQUID_UPDATE_FREQUENCY)
	{
		return;
	}
	elapsed -= POWDER_LIQUID_UPDATE_FREQUENCY;

	/*	Two passes are so that particles arent moved more than once in one update.
		This does provide a downside though, not every particle is guaranteed to be moved. */

	/* left pass */
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			if (POWDER_IS_LIQUID(powder_get(x, y)) && powder_is_grounded(x, y, TYPE_GROUND | TYPE_SAND) && rand() % 2)
			{
				if (POWDER_IS_AIR(powder_get(x - 1, y)))
				{
					powder_set(x, y, TYPE_AIR);
					powder_set(x - 1, y, TYPE_WATER);
				}
			}
		}
	}
	/* right pass */
	for (int x = SCREEN_WIDTH; x >= 0; x--)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			if (POWDER_IS_LIQUID(powder_get(x, y)) && powder_is_grounded(x, y, TYPE_GROUND | TYPE_SAND) && rand() % 2)
			{
				if (POWDER_IS_AIR(powder_get(x + 1, y)))
				{
					powder_set(x, y, TYPE_AIR);
					powder_set(x + 1, y, TYPE_WATER);
				}
			}
		}
	}
}

void powder_update(double delta)
{
	powder_update_spread();
	powder_update_gravity();
	powder_update_liquid(delta);
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
			case TYPE_WATER:
				screen_set_pixel(x, y, COLOR_LIGHT_BLUE);
				break;
			case TYPE_DEBUG:
				screen_set_pixel(x, y, COLOR_LIGHT_RED);
				break;
			}
		}
	}
}

void powder_key_clicked(char key)
{
	if (key >= '0' && key <= '3')
	{
		key -= '0';
		current = 1 << key;
	}
}

static inline void powder_set_rectangle(int x, int y, int wx, int wy, powder_type_t type)
{
	for (int xo = 0; xo < wx; xo++)
	{
		for (int yo = 0; yo < wy; yo++)
		{
			powder_set(x + xo, y + yo, type);
		}
	}
}

void powder_mouse_primary_down(int x, int y)
{
	if (POWDER_IS_AIR(powder_get(x, y)))
	{
		powder_set_rectangle(x - powder_brush_size / 2, y - powder_brush_size / 2, powder_brush_size, powder_brush_size, current);
	}
}

void powder_mouse_aux_down(int x, int y)
{
	powder_set_rectangle(x - powder_brush_size / 2, y - powder_brush_size / 2, powder_brush_size, powder_brush_size, TYPE_AIR);
}