/*
	main.c ~ RL
*/

#include <assert.h>
#include <stdbool.h>
#include <Windows.h>

#define RETURN_IF_CONDITION(cond) do { if (cond) return __LINE__; } while (0)

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

static CHAR_INFO screen[SCREEN_WIDTH * SCREEN_HEIGHT];
static HANDLE input, output;

inline void screen_set_pixel(int x, int y, screen_color_t color)
{
	screen[x + y * SCREEN_WIDTH].Attributes = color << 4;
}

inline void screen_set_rect(int x, int y, int wx, int wy, screen_color_t color)
{
	for (int ox = 0; ox < wx; ox++)
	{
		for (int oy = 0; oy < wy; oy++)
		{
			screen_set_pixel(ox + x, oy + y, color);
		}
	}
}

inline void screen_invalidate(void)
{
	SMALL_RECT rect = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
	if (FALSE == WriteConsoleOutputW(output, screen, (COORD) { SCREEN_WIDTH, SCREEN_HEIGHT }, (COORD) { 0, 0 }, &rect))
	{
		exit(-1);
	}
}

int main()
{
	output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	RETURN_IF_CONDITION(INVALID_HANDLE_VALUE == output);
	RETURN_IF_CONDITION(FALSE == SetConsoleActiveScreenBuffer(output));
	RETURN_IF_CONDITION(FALSE == SetConsoleMode(output, 0));

	input = GetStdHandle(STD_INPUT_HANDLE);
	RETURN_IF_CONDITION(INVALID_HANDLE_VALUE == input || NULL == input);
	RETURN_IF_CONDITION(FALSE == SetConsoleMode(input, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));

	{
		CONSOLE_CURSOR_INFO cci;
		RETURN_IF_CONDITION(FALSE == GetConsoleCursorInfo(output, &cci));
		cci.bVisible = FALSE;
		RETURN_IF_CONDITION(FALSE == SetConsoleCursorInfo(output, &cci));
	}

	{
		CONSOLE_FONT_INFOEX cfi = { sizeof cfi };
		RETURN_IF_CONDITION(FALSE == GetCurrentConsoleFontEx(output, FALSE, &cfi));
		cfi.dwFontSize.X = SCREEN_PIXEL_SIZE - 1; /* TO DO: why does this make pixels the right size? */
		cfi.dwFontSize.Y = SCREEN_PIXEL_SIZE;
		cfi.FontFamily = FF_DONTCARE;
		cfi.nFont = 0;
		memset(cfi.FaceName, 0, sizeof cfi.FaceName);
		RETURN_IF_CONDITION(FALSE == SetCurrentConsoleFontEx(output, FALSE, &cfi));
	}

	{
		CONSOLE_SCREEN_BUFFER_INFOEX csbi = { sizeof csbi };
		RETURN_IF_CONDITION(FALSE == GetConsoleScreenBufferInfoEx(output, &csbi));

		csbi.dwMaximumWindowSize.X = SCREEN_WIDTH;
		csbi.dwMaximumWindowSize.Y = SCREEN_HEIGHT;
		csbi.dwSize = csbi.dwMaximumWindowSize;

		COLORREF table[16] =
		{
			RGB(0, 0, 0),
			RGB(0, 0, 128),
			RGB(0, 128, 0),
			RGB(0, 128, 128),
			RGB(128, 0, 0),
			RGB(128, 0, 128),
			RGB(128, 128, 0),
			RGB(192, 192, 192),
			RGB(128, 128, 128),
			RGB(0, 0, 255),
			RGB(0, 255, 0),
			RGB(0, 255, 255),
			RGB(255, 0, 0),
			RGB(255, 0, 255),
			RGB(255, 255, 0),
			RGB(255, 255, 255),
		};

		memcpy(csbi.ColorTable, table, sizeof table);

		RETURN_IF_CONDITION(FALSE == SetConsoleScreenBufferInfoEx(output, &csbi));
	}

	RETURN_IF_CONDITION(FALSE == SetConsoleTitleW(L"QuickPowder"));

	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		screen[i].Char.UnicodeChar = L' ';
		screen[i].Attributes = COLOR_BLACK << 4;
	}

	screen_set_rect(14, 14, 12, 4, COLOR_WHITE);
	for (screen_color_t i = 0; i < 8; i++)
	{
		screen_set_pixel(16 + i, 16, i);
	}

	for (screen_color_t i = 8; i < 16; i++)
	{
		screen_set_pixel(20 + i, 16, i);
	}
	
	screen_invalidate();

	bool running = true;
	INPUT_RECORD record;
	DWORD records_read_len;
	do
	{
		RETURN_IF_CONDITION(FALSE == ReadConsoleInputW(input, &record, 1, &records_read_len));
		assert(1 == records_read_len);

		if (record.EventType == KEY_EVENT && record.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
		{
			running = false;
		}

	} while (running);

	/* don't close STD input handle */
	CloseHandle(output);

	return 0;
}