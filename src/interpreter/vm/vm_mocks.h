/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_mocks.h: a way to overide functions like execvp to be able to unit test vm.c */

#pragma once

// clang-format off
#ifdef NCSH_VM_TEST
    int execvp_mock(char* arg, char** args, int res) {
        (void)arg;
        (void)args;
        return res;
    }

    int waitpid_mock(pid_t pid, int* status, int w) {
        (void)status;
        (void)w;
        return pid;
    }

#   define fork() (pid_t)1
#   ifdef NCSH_VM_TEST_EXEC_FAILURE
#       define execvp(arg, args) execvp_mock(arg, args, -1)
#   else
#       define execvp(arg, args) execvp_mock(arg, args, 0)
#   endif /* NCSH_VM_TEST_EXEC_FAILURE */
#   define waitpid(pid, status, w) waitpid_mock(pid, status, w)
#endif /* NCSH_VM_TEST */
// clang-format on
