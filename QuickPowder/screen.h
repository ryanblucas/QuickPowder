/*
	screen.h ~ RL
*/

#pragma once

#define SCREEN_PIXEL_SIZE	8
#define SCREEN_WIDTH		(800 / SCREEN_PIXEL_SIZE)
#define SCREEN_HEIGHT		(600 / SCREEN_PIXEL_SIZE)

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

void screen_set_pixel(int x, int y, screen_color_t color);
void screen_set_rect(int x, int y, int wx, int wy, screen_color_t color);
void screen_format(const char* fmt, ...);