/*
	main.c ~ RL
*/

#include <assert.h>
#include <stdbool.h>
#include <Windows.h>
#include <strsafe.h>

#define SCREEN_PIXEL_SIZE	8
#define SCREEN_WIDTH		(800 / SCREEN_PIXEL_SIZE)
#define SCREEN_HEIGHT		(600 / SCREEN_PIXEL_SIZE)

#define __STR2(s) __STR(s)
#define __STR(s) #s
#define RUNTIME_ASSERT(func) ((func) || (debug_format(#func " failed at line " __STR2(__LINE__) ".\n"), exit(-1), false))
#define RUNTIME_ASSERT_WIN32(func) ((func) || (debug_format(#func " failed at line " __STR2(__LINE__) " with error code %i.\n", GetLastError()), exit(-1), false))

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

void debug_format(const char* fmt, ...)
{
	static char* current_buffer = NULL;
	static int size;

	if (!current_buffer)
	{
		size = 256;
		current_buffer = malloc(size * sizeof * current_buffer);
		if (!current_buffer)
		{
			exit(1); /* The program most certainly couldn't run if it can't allocate 256 bytes */
		}
	}

	va_list list;
	va_start(list, fmt);
	while (StringCbVPrintfA(current_buffer, size, fmt, list) == STRSAFE_E_INSUFFICIENT_BUFFER)
	{
		free(current_buffer);
		size *= 2;
		current_buffer = malloc(size * sizeof * current_buffer);
		if (!current_buffer)
		{
			exit(1);
		}
	}
	va_end(list);

	OutputDebugStringA(current_buffer);
}

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

static void screen_invalidate(void)
{
	SMALL_RECT rect = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
	RUNTIME_ASSERT_WIN32(WriteConsoleOutputW(output, screen, (COORD) { SCREEN_WIDTH, SCREEN_HEIGHT }, (COORD) { 0, 0 }, &rect));
}

static void screen_initialize_cursor(void)
{
	CONSOLE_CURSOR_INFO cci;
	RUNTIME_ASSERT_WIN32(TRUE == GetConsoleCursorInfo(output, &cci));
	cci.bVisible = FALSE;
	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleCursorInfo(output, &cci));
}

static void screen_initialize_font(void)
{
	CONSOLE_FONT_INFOEX cfi = { sizeof cfi };
	RUNTIME_ASSERT_WIN32(TRUE == GetCurrentConsoleFontEx(output, FALSE, &cfi));
	cfi.dwFontSize.X = SCREEN_PIXEL_SIZE - 1; /* TO DO: why does this make pixels the right size? */
	cfi.dwFontSize.Y = SCREEN_PIXEL_SIZE;
	cfi.FontFamily = FF_DONTCARE;
	cfi.nFont = 0;
	memset(cfi.FaceName, 0, sizeof cfi.FaceName);
	RUNTIME_ASSERT_WIN32(TRUE == SetCurrentConsoleFontEx(output, FALSE, &cfi));
}

static void screen_initialize_output_buffer(void)
{
	CONSOLE_SCREEN_BUFFER_INFOEX csbi = { sizeof csbi };
	RUNTIME_ASSERT_WIN32(TRUE == GetConsoleScreenBufferInfoEx(output, &csbi));

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

	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleScreenBufferInfoEx(output, &csbi));
}

int main()
{
	output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	RUNTIME_ASSERT_WIN32(INVALID_HANDLE_VALUE != output);
	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleActiveScreenBuffer(output));
	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleMode(output, 0));

	input = GetStdHandle(STD_INPUT_HANDLE);
	RUNTIME_ASSERT_WIN32(INVALID_HANDLE_VALUE != input && NULL != input);
	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleMode(input, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));

	screen_initialize_cursor();
	screen_initialize_font();
	screen_initialize_output_buffer();

	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleTitleW(L"QuickPowder"));

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
		RUNTIME_ASSERT_WIN32(TRUE == ReadConsoleInputW(input, &record, 1, &records_read_len));
		assert(1 == records_read_len);

		switch (record.EventType)
		{
		case KEY_EVENT:
		{
			if (record.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
			{
				running = false;
			}
			break;
		}
		case WINDOW_BUFFER_SIZE_EVENT:
		{
			RECT fitted = (RECT){ .right = SCREEN_WIDTH * SCREEN_PIXEL_SIZE, .bottom = SCREEN_HEIGHT * SCREEN_PIXEL_SIZE };
			assert(AdjustWindowRectEx(&fitted, GetWindowLong(GetConsoleWindow(), GWL_STYLE), FALSE, GetWindowLong(GetConsoleWindow(), GWL_EXSTYLE)));
			RUNTIME_ASSERT_WIN32(TRUE == SetWindowPos(GetConsoleWindow(), NULL, 0, 0, fitted.right - fitted.left, fitted.bottom - fitted.top, SWP_NOMOVE));

			screen_invalidate();
			screen_initialize_cursor();

			break;
		}
		}

	} while (running);

	/* don't close STD input handle */
	CloseHandle(output);

	return 0;
}