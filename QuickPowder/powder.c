/*
	powder.c ~ RL
*/

#include "powder.h"
#include "screen.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define POWDER_INDEX(x, y) (canvas[(x) + (y) * SCREEN_WIDTH])

static powder_type_t canvas[SCREEN_WIDTH * SCREEN_HEIGHT];
static powder_type_t current;

int powder_brush_size = 2;

void powder_init(unsigned int seed)
{
	srand(seed);
	for (int i = 0; i < sizeof canvas / sizeof * canvas; i++)
	{
		canvas[i] = TYPE_AIR;
	}
	powder_key_clicked('2'); /* sand */
}

static inline powder_type_t powder_get(int x, int y)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return TYPE_GROUND;
	}
	return POWDER_INDEX(x, y);
}

static inline void powder_set(int x, int y, powder_type_t type)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return;
	}
	POWDER_INDEX(x, y) = type;
}

static inline bool powder_is_grounded(int x, int y, powder_type_t mask)
{
	powder_type_t query = POWDER_INDEX(x, y);
	for (; POWDER_INDEX(x, y) == query && y < SCREEN_HEIGHT; y++);
	return powder_get(x, y) & mask;
}

static void powder_update_spread(void)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 1; y > 0; y--)
		{
			powder_type_t curr = POWDER_INDEX(x, y), above = POWDER_INDEX(x, y - 1);
			if (TYPE_SAND == curr && TYPE_SAND == above && powder_is_grounded(x, y, TYPE_GROUND))
			{
				powder_type_t left = powder_get(x - 1, y),
					right = powder_get(x + 1, y);
				if (!POWDER_IS_SOLID(left))
				{
					POWDER_INDEX(x, y - 1) = TYPE_AIR;
					powder_set(x - 1, y, TYPE_SAND);
				}
				else if (!POWDER_IS_SOLID(right))
				{
					POWDER_INDEX(x, y - 1) = TYPE_AIR;
					powder_set(x + 1, y, TYPE_SAND);
				}
			}
			else if (TYPE_WATER == curr && TYPE_WATER == above && powder_is_grounded(x, y, TYPE_GROUND | TYPE_SAND))
			{
				int lx, rx;
				for (lx = x; powder_get(lx, y) == TYPE_WATER; lx--);
				for (rx = x; powder_get(rx, y) == TYPE_WATER; rx++);
				int lwx = x - lx,
					rwx = rx - x;
				if (lwx > rwx && powder_get(rx, y) == TYPE_AIR)
				{
					powder_set(rx, y, TYPE_WATER);
					POWDER_INDEX(x, y - 1) = TYPE_AIR;
				}
				else if (lwx <= rwx && powder_get(lx, y) == TYPE_AIR)
				{
					powder_set(lx, y, TYPE_WATER);
					POWDER_INDEX(x, y - 1) = TYPE_AIR;
				}
			}
		}
	}
}

static void powder_update_gravity(void)
{
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--)
	{
		for (int y = SCREEN_HEIGHT - 2; y >= 0; y--)
		{
			powder_type_t curr = POWDER_INDEX(x, y), down = POWDER_INDEX(x, y + 1);
			if (curr & (TYPE_SAND | TYPE_WATER) && !POWDER_IS_SOLID(down))
			{
				POWDER_INDEX(x, y) = down;
				POWDER_INDEX(x, y + 1) = curr;
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
			if (POWDER_IS_LIQUID(POWDER_INDEX(x, y)) && powder_is_grounded(x, y, TYPE_GROUND | TYPE_SAND) && rand() % 2)
			{
				if (POWDER_IS_AIR(powder_get(x - 1, y)))
				{
					POWDER_INDEX(x, y) = TYPE_AIR;
					POWDER_INDEX(x - 1, y) = TYPE_WATER;
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
					POWDER_INDEX(x, y) = TYPE_AIR;
					POWDER_INDEX(x + 1, y) = TYPE_WATER;
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
	screen_clear(COLOR_BLACK);
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		/* this switch statement is faster than an array lookup? */
		switch (canvas[i])
		{
		case TYPE_SAND:
			screen[i].Attributes = COLOR_LIGHT_YELLOW << 4;
			break;
		case TYPE_GROUND:
			screen[i].Attributes = COLOR_DARK_GRAY << 4;
			break;
		case TYPE_WATER:
			screen[i].Attributes = COLOR_LIGHT_BLUE << 4;
			break;
		case TYPE_DEBUG:
			screen[i].Attributes = COLOR_LIGHT_RED << 4;
			break;
		}
	}
}

void powder_key_clicked(char key)
{
	if (key >= '0' && key <= '3')
	{
		key -= '0';
		current = 1 << key;
		switch (current)
		{
		case TYPE_SAND:
			screen_set_caption("Sand selected");
			break;
		case TYPE_GROUND:
			screen_set_caption("Ground selected");
			break;
		case TYPE_WATER:
			screen_set_caption("Water selected");
			break;
		}
	}
}

static inline void powder_set_rectangle(int x, int y, int wx, int wy, powder_type_t type)
{
	wx = min(x + wx, SCREEN_WIDTH) - x;
	x = max(x, 0);
	wy = min(y + wy, SCREEN_HEIGHT) - y;
	y = max(y, 0);

	for (int xo = 0; xo < wx; xo++)
	{
		for (int yo = 0; yo < wy; yo++)
		{
			POWDER_INDEX(x + xo, y + yo) = type;
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