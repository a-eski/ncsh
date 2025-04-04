#pragma once

#include <setjmp.h>
#include <signal.h>

extern sig_atomic_t vm_child_pid;
extern jmp_buf env;
