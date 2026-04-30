#pragma once

// Command type
#include "command.h"

Command parseInput(const char* input);

void freeCommand(Command* command);