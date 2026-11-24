#pragma once

#include <string>

// The lines about the build and the machine for the top of the report.
// Results from different machines or builds can't be compared without them.
std::string environment_text();
