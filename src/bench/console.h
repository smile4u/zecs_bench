#pragma once

// True when stdout is a terminal that shows ANSI colors. It is false when
// the output goes to a file, so the file gets plain markdown.
// NO_COLOR (https://no-color.org) and TERM=dumb turn the colors off.
// On Windows it also turns on ANSI colors in the console.
bool console_has_colors();
