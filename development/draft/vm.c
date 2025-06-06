
/* Background Jobs */
// Implementation not working, still experimenting...
/*[[nodiscard]]
int vm_background_job_run(Args* rst args, Processes* rst processes,
                          Tokens* rst tokens)
{
    assert(processes);
    (void)tokens;

    int execvp_result = NCSH_COMMAND_NONE;
    pid_t pid = fork();

    if (pid < 0) {
        perror(RED "ncsh: Error when forking process" RESET);
        fflush(stdout);
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    else if (pid == 0) { // runs in the child process
        setsid();
        signal(SIGCHLD, SIG_DFL); // Restore default handler in child

        // Redirect standard input, output, and error
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        char cmds[PARSER_TOKENS_LIMIT][NCSH_MAX_INPUT];
        Arg* arg = args->head->next;
        for (size_t i = 0; i < args->count && arg; ++i) {
            memcpy(cmds[i], arg->val, arg->len);
            arg = arg->next;
        }

        if ((execvp_result = execvp(cmds[0], (char**)cmds)) == EXECVP_FAILED) {
            perror(RED "ncsh: Could not run command" RESET);
            fflush(stdout);
            kill(getpid(), SIGTERM);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }
    else {
        signal(SIGCHLD, SIG_IGN); // Prevent zombie processes
        size_t job_number = ++processes->job_number;
        printf("job [%zu] pid [%d]\n", job_number, pid);
        processes->pids[job_number - 1] = pid;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}*/

/*void vm_background_jobs_check(Processes* rst processes)
{
    assert(processes);
    (void)processes;

    for (size_t i = 0; i < processes->job_number; ++i) {
        pid_t waitpid_result;
        while (1) {
            int status = 0;
            waitpid_result = waitpid(processes->pids[0], &status, WUNTRACED);

            // check for errors
            if (waitpid_result == -1) {
                // ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags
                if (errno == EINTR) {
                    continue;
                }

                perror(RED "ncsh: Error waiting for job child process to exit" RESET);
                break;
            }
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status)) {
                    fprintf(stderr, "ncsh: Command child process returned with status %d\n", WEXITSTATUS(status));
                }
#ifdef NCSH_DEBUG
                else {
                    fprintf(stderr, "ncsh: Command child process exited successfully.\n");
                }
#endif // NCSH_DEBUG
            }
        }
    }
}*/
