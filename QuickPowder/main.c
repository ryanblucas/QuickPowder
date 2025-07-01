/*
	main.c ~ RL
*/

#include <assert.h>
#include <stdbool.h>
#include <Windows.h>

#define RETURN_IF_CONDITION(cond) do { if (cond) return __LINE__; } while (0)

static HANDLE input, output;

int main()
{
	output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	RETURN_IF_CONDITION(INVALID_HANDLE_VALUE == output);
	RETURN_IF_CONDITION(FALSE == SetConsoleActiveScreenBuffer(output));

	input = GetStdHandle(STD_INPUT_HANDLE);
	RETURN_IF_CONDITION(INVALID_HANDLE_VALUE == input || NULL == input);

	bool running = true;
	INPUT_RECORD record;
	DWORD records_read_len;
	do
	{
		RETURN_IF_CONDITION(FALSE == ReadConsoleInputW(input, &record, 1, &records_read_len));
		assert(1 == records_read_len);

		if (record.EventType == KEY_EVENT)
		{
			running = false;
		}

	} while (running);

	/* don't close STD input handle */
	CloseHandle(output);

	return 0;
}