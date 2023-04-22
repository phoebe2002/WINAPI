#define NOMINMAX
#include <windows.h>
#include "bouncy.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE /*prevInstance*/, LPWSTR /*command_line*/, int show_command)
{
	bouncy app{ instance };
	return app.run(show_command);
}