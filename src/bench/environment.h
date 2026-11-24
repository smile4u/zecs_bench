#pragma once

#include <string>

// The system and the CPU, for example "macOS arm64, Apple M1 Max".
// The report writes it at the top in big letters.
std::string platform_text();

// The lines about the versions, the build and the machine for the top of the report.
// Results from different machines or builds can't be compared without them.
std::string environment_text();
