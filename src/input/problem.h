#pragma once

#include "../kv.h"

#define I_PROBLEM_UNKNOWN_OPERATION "Unknown operation"
#define I_PROBLEM_BAD_TERMINATION "Bad termination, commands must end with one '\\n'"
#define I_PROBLEM_INPUT_TOO_LONG "Input length too long, max " TOSTRING(MAX_LINE_LEN)
#define I_PROBLEM_KEY_TOO_LONG "Key length too long, max " TOSTRING(MAX_KEY_LEN)
#define I_PROBLEM_VAL_TOO_LONG "Value length too long, max " TOSTRING(MAX_VAL_LEN)
#define I_PROBLEM_IO "Error reading from socket"