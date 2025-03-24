/* experiment with writing a shell for windows */

#include <windows.h>
#include <conio.h>
#include <stdio.h>

int main(void) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwMode;

	// Get the current input mode
	GetConsoleMode(hStdin, &dwMode);

	// Set the console to non-canonical mode
	dwMode &= ~ENABLE_ECHO_INPUT; // Disable echoing
	dwMode &= ~ENABLE_LINE_INPUT; // Disable line-based input
	SetConsoleMode(hStdin, dwMode);


	printf("> ");

	char ch;
	while (1) {
		if (_kbhit()) { // Check if a key has been pressed
			ch = _getch();

			if (ch == '\x1B') { // Check for Escape key
				break;
			} else if (ch == '\r') { // Enter key
				printf("\n");
				printf("> ");
			} else {
				printf("%c", ch); // Echo the character
			}
		}
	}

	// Restore the original console mode
	SetConsoleMode(hStdin, dwMode);
	return 0;
}
