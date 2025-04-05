#pragma once

#define _POSIX_C_SOURCE 200809L

#include <setjmp.h>
#include <signal.h>

extern jmp_buf env;
extern sig_atomic_t vm_child_pid;
