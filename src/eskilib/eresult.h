/* Copyright eskilib by Alex Eski 2024 */

#pragma once

enum eresult {
    E_FAILURE_BAD_STRING = -8,
    E_FAILURE_NOT_FOUND = -7,
    E_FAILURE_ZERO_LENGTH = -6,
    E_FAILURE_UNDERFLOW_PROTECTION = -5,
    E_FAILURE_OVERFLOW_PROTECTION = -4,
    E_FAILURE_FILE_OP = -3,
    E_FAILURE_MALLOC = -2,
    E_FAILURE_NULL_REFERENCE = -1,
    E_FAILURE = 0,
    E_SUCCESS = 1,
    E_NO_OP = 2,
    E_NO_OP_MAX_LIMIT_REACHED = 3
};
