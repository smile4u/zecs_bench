// MSVC warns that std::getenv isn't safe and wants _dupenv_s. This file only
// reads two variables and doesn't keep the pointers, so std::getenv is enough.
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "bench/console.h"

#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

bool console_has_colors()
{
    if (std::getenv("NO_COLOR"))
        return false;

    const char* term = std::getenv("TERM");
    if (term && std::strcmp(term, "dumb") == 0)
        return false;

#ifdef _WIN32
    // GetConsoleMode() fails when stdout isn't a console.
    const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(out, &mode))
        return false;
    return SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}
