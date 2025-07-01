/*
	screen.c ~ RL
*/

#include "screen.h"

#include <assert.h>
#include "powder.h"
#include <stdbool.h>
#include <strsafe.h>
#include <Windows.h>

#define SCREEN_TITLE		L"QuickPowder"
#define SCREEN_TARGET_DELTA	(1.0 / 144)

#define __STR2(s) __STR(s)
#define __STR(s) #s
#define RUNTIME_ASSERT(func) ((func) || (screen_format(#func " failed at line " __STR2(__LINE__) ".\n"), exit(-1), false))
#define RUNTIME_ASSERT_WIN32(func) ((func) || (screen_format(#func " failed at line " __STR2(__LINE__) " with error code %i.\n", GetLastError()), exit(-1), false))

#define MOUSE_1 0x01
#define MOUSE_2 0x02

typedef struct mouse
{
	int mask, pmask;
	int x, y;
	int px, py;
} mouse_t;

static CHAR_INFO screen[SCREEN_WIDTH * SCREEN_HEIGHT];
static HANDLE input, output;

void screen_format(const char* fmt, ...)
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

void screen_set_pixel(int x, int y, screen_color_t color)
{
	screen[x + y * SCREEN_WIDTH].Attributes = color << 4;
}

void screen_set_rect(int x, int y, int wx, int wy, screen_color_t color)
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

static void screen_handle_mouse(mouse_t* mouse, MOUSE_EVENT_RECORD mer)
{
	if (mer.dwEventFlags == 0)
	{
		mouse->mask = !!(mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) | (!!(mer.dwButtonState & RIGHTMOST_BUTTON_PRESSED) << 1);
	}
	mouse->x = mer.dwMousePosition.X;
	mouse->y = mer.dwMousePosition.Y;
}

static void screen_update_mouse(mouse_t* mouse)
{
	if (mouse->mask & MOUSE_1)
	{
		/* credit: https://zingl.github.io/bresenham.html */
		int dx = abs(mouse->x - mouse->px),
			sx = mouse->px < mouse->x ? 1 : -1;
		int dy = -abs(mouse->y - mouse->py),
			sy = mouse->py < mouse->y ? 1 : -1;
		int error = dx + dy;

		int x = mouse->px, y = mouse->py;
		while (true)
		{
			powder_mouse_primary_down(x, y);
			if (x == mouse->x && y == mouse->y)
			{
				break;
			}
			int error2 = error * 2;
			if (error2 >= dy)
			{
				error += dy;
				x += sx;
			}
			if (error2 <= dx)
			{
				error += dx;
				y += sy;
			}
		}
	}
	if (mouse->mask & MOUSE_2 && mouse->pmask ^ MOUSE_2)
	{
		powder_mouse_aux_down(mouse->x, mouse->y);
	}
	mouse->px = mouse->x;
	mouse->py = mouse->y;
	mouse->pmask = mouse->mask;
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

	RUNTIME_ASSERT_WIN32(TRUE == SetConsoleTitleW(SCREEN_TITLE));

	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		screen[i].Char.UnicodeChar = L' ';
		screen[i].Attributes = COLOR_BLACK << 4;
	}

	bool running = true;

	LARGE_INTEGER frequency, curr, prev;
	LARGE_INTEGER fps_elapsed = { 0 }, tick_elapsed = { 0 };
	int fps_samples = 0;
	RUNTIME_ASSERT_WIN32(TRUE == QueryPerformanceFrequency(&frequency));

	RUNTIME_ASSERT_WIN32(TIMERR_NOERROR == timeBeginPeriod(1));

	mouse_t mouse = { 0 };

	RUNTIME_ASSERT_WIN32(TRUE == QueryPerformanceCounter(&prev));
	do
	{
		RUNTIME_ASSERT_WIN32(TRUE == QueryPerformanceCounter(&curr));

		INPUT_RECORD record;
		DWORD records_read_len;
		RUNTIME_ASSERT_WIN32(TRUE == PeekConsoleInputW(input, &record, 1, &records_read_len));
		if (1 == records_read_len)
		{
			/* removes from buffer */
			RUNTIME_ASSERT_WIN32(TRUE == ReadConsoleInputW(input, &record, 1, &records_read_len));
			switch (record.EventType)
			{
			case KEY_EVENT:
			{
				KEY_EVENT_RECORD ker = record.Event.KeyEvent;
				if (ker.wVirtualKeyCode == VK_ESCAPE)
				{
					running = false;
				}
				else if (ker.uChar.AsciiChar >= '0' && ker.uChar.AsciiChar <= '9' && ker.bKeyDown == TRUE)
				{
					powder_key_clicked(ker.uChar.AsciiChar);
				}
				break;
			}
			case MOUSE_EVENT:
			{
				screen_handle_mouse(&mouse, record.Event.MouseEvent);
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
		}

		LONGLONG delta_long = curr.QuadPart - prev.QuadPart;
		tick_elapsed.QuadPart += delta_long;

		double update_delta = (double)tick_elapsed.QuadPart / frequency.QuadPart;
		if (update_delta > POWDER_TICK_RATE)
		{
			screen_update_mouse(&mouse);
			powder_update(update_delta);
			tick_elapsed.QuadPart -= (LONGLONG)(POWDER_TICK_RATE * frequency.QuadPart);
		}

		powder_render((double)(delta_long) / frequency.QuadPart);
		screen_invalidate();

		fps_elapsed.QuadPart += delta_long;

		fps_samples++;
		if (fps_elapsed.QuadPart >= frequency.QuadPart)
		{
			double fps = 1.0 / ((double)(fps_elapsed.QuadPart / fps_samples) / frequency.QuadPart);
			wchar_t title_buf[128];
			swprintf_s(title_buf, sizeof title_buf / sizeof * title_buf, SCREEN_TITLE L", FPS %f", fps);
			RUNTIME_ASSERT_WIN32(TRUE == SetConsoleTitleW(title_buf));
			fps_elapsed.QuadPart = 0;
			fps_samples = 0;
		}

		LARGE_INTEGER end;
		RUNTIME_ASSERT_WIN32(TRUE == QueryPerformanceCounter(&end));

		/* not perfect, but this is fine for a project like this */
		double rest = SCREEN_TARGET_DELTA - (double)(end.QuadPart - curr.QuadPart) / frequency.QuadPart;
		if (rest > 0.0F)
		{
			Sleep((DWORD)(rest * 1000));
		}

		prev = curr;

	} while (running);

	/* don't close STD input handle */
	CloseHandle(output);
	RUNTIME_ASSERT_WIN32(TIMERR_NOERROR == timeEndPeriod(1));

	return 0;
}