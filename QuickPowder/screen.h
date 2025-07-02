/*
	screen.h ~ RL
*/

#pragma once

#include <immintrin.h>
#include <Windows.h>

#define SCREEN_PIXEL_SIZE	4
#define SCREEN_WIDTH		(800 / SCREEN_PIXEL_SIZE)
#define SCREEN_HEIGHT		(600 / SCREEN_PIXEL_SIZE)

extern CHAR_INFO screen[SCREEN_WIDTH * SCREEN_HEIGHT];

typedef enum screen_color
{
	COLOR_BLACK,
	COLOR_DARK_BLUE,
	COLOR_DARK_GREEN,
	COLOR_DARK_CYAN,
	COLOR_DARK_RED,
	COLOR_DARK_PURPLE,
	COLOR_DARK_YELLOW,
	COLOR_LIGHT_GRAY,
	COLOR_DARK_GRAY,
	COLOR_LIGHT_BLUE,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_CYAN,
	COLOR_LIGHT_RED,
	COLOR_LIGHT_PURPLE,
	COLOR_LIGHT_YELLOW,
	COLOR_WHITE
} screen_color_t;

extern inline void screen_set_pixel(int x, int y, screen_color_t color)
{
	screen[x + y * SCREEN_WIDTH].Attributes = color << 4;
}

extern inline void screen_set_rect(int x, int y, int wx, int wy, screen_color_t color)
{
	for (int ox = 0; ox < wx; ox++)
	{
		for (int oy = 0; oy < wy; oy++)
		{
			screen[x + ox + (y + oy) * SCREEN_WIDTH].Attributes = color << 4;
		}
	}
}

extern inline void screen_clear(screen_color_t color)
{
	CHAR_INFO desired = { .Char.AsciiChar = ' ', .Attributes = color << 4};
	__m256i curr = _mm256_set1_epi32(*(int*)&desired); /* UB, but who cares */
	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT) / 8; i++)
	{
		_mm256_storeu_si256((__m256i*)(&screen[i * 8]), curr);
	}
}

void screen_format(const char* fmt, ...);
void screen_set_caption(const char* str);