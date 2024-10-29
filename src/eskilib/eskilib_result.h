// Copyright (c) eskilib by Alex Eski 2024

#ifndef eskilib_types_h
#define eskilib_types_h

enum eskilib_Result {
	E_ZERO_LENGTH = -6,
	E_UNDERFLOW_PROTECTION = -5,
	E_OVERFLOW_PROTECTION = -4,
	E_FAILURE_FILE_OP = -3,
	E_FAILURE_MALLOC = -2,
	E_FAILURE_NULL_REFERENCE = -1,
	E_FAILURE = 0,
	E_SUCCESS = 1,
	E_MAX_LIMIT_REACHED = 2
};

#endif // !eskilib_types_h

