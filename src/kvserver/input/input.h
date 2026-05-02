#pragma once

// Command type
#include "../../common/command.h"

Command parseInput(const char* input, char delimiter, char terminator);

char* readLine(int fd);