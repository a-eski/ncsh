// Parser goto dispatch experiment
/*
#define REDIRECT(tok, str)                                                                                             \
    assert(tok->next && tok->next->val);                                                                               \
    if (!tok->next || !tok->next->val)                                                                                 \
        goto NEXT_INSTRUCTION;                                                                                         \
    str = tok->next->val;                                                                                              \
    tok = NULL;                                                                                                        \
    if (prev)                                                                                                          \
        prev->next = NULL;


[[nodiscard]]
int parser_ops_process(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(toks && toks->head);
    assert(scratch);

    Token* tok = toks->head->next;
    Token* prev = NULL;
    toks->data.is_background_job = false;
    Logic_Result result;

    static void* dispatch_table[] = {
        &&NEXT_INSTRUCTION,                           // OP_NONE = 0
        &&NEXT_INSTRUCTION,                           // OP_CONSTANT = 1
        &&PIPE_LABEL,                                 // OP_PIPE = 2
        &&STDOUT_REDIRECTION_LABEL,                   // OP_STDOUT_REDIRECTION = 3
        &&STDOUT_REDIRECTION_APPEND_LABEL,            // OP_STDOUT_REDIRECTION_APPEND = 4
        &&STDIN_REDIRECTION_LABEL,                    // OP_STDIN_REDIRECTION = 5
        &&STDIN_REDIRECTION_APPEND_LABEL,             // OP_STDIN_REDIRECTION_APPEND = 6
        &&STDERR_REDIRECTION_LABEL,                   // OP_STDERR_REDIRECTION = 7
        &&STDERR_REDIRECTION_APPEND_LABEL,            // OP_STDERR_REDIRECTION_APPEND = 8
        &&STDOUT_AND_STDERR_REDIRECTION_LABEL,        // OP_STDOUT_AND_STDERR_REDIRECTION = 9
        &&STDOUT_AND_STDERR_REDIRECTION_APPEND_LABEL, // OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10
        &&BACKGROUND_JOB_LABEL,                       // OP_BACKGROUND_JOB = 11
        &&NEXT_INSTRUCTION,                           // OP_AND = 12
        &&NEXT_INSTRUCTION,                           // OP_OR = 13
        &&NEXT_INSTRUCTION,                           // OP_ADD = 14
        &&NEXT_INSTRUCTION,                           // OP_SUBTRACT = 15
        &&NEXT_INSTRUCTION,                           // OP_MULTIPLY = 16
        &&NEXT_INSTRUCTION,                           // OP_DIVIDE = 17
        &&NEXT_INSTRUCTION,                           // OP_MODULO = 18
        &&NEXT_INSTRUCTION,                           // OP_EXPONENTIATION = 19
        &&NEXT_INSTRUCTION,                           // OP_MATH_EXPRESSION_START = 20
        &&NEXT_INSTRUCTION,                           // OP_MATH_EXPRESSION_END = 21
        &&NEXT_INSTRUCTION,                           // OP_VARIABLE = 22
        &&ASSIGNMENT_LABEL,                           // OP_ASSIGNMENT = 23
        &&NEXT_INSTRUCTION,                           // OP_TRUE = 24
        &&NEXT_INSTRUCTION,                           // OP_FALSE = 25
        &&NEXT_INSTRUCTION,                           // OP_HOME_EXPANSION = 26
        &&NEXT_INSTRUCTION,                           // OP_GLOB_EXPANSION = 27
        &&NEXT_INSTRUCTION,                           // OP_CONDITION_START = 28
        &&NEXT_INSTRUCTION,                           // OP_CONDITION_END = 29
        &&IF_LABEL,                                   // OP_IF = 30
        &&NEXT_INSTRUCTION,                           // OP_ELSE = 31
        &&NEXT_INSTRUCTION,                           // OP_ELIF = 32
        &&NEXT_INSTRUCTION,                           // OP_THEN = 33
        &&FI_LABEL,                                   // OP_FI = 34
        &&NEXT_INSTRUCTION,                           // OP_EQUALS = 35
        &&NEXT_INSTRUCTION,                           // OP_LESS_THAN = 36
        &&NEXT_INSTRUCTION,                           // OP_GREATER_THAN = 37
    };

    while (tok) {
        goto* dispatch_table[tok->op];

    PIPE_LABEL:
        ++toks->data.number_of_pipe_commands;
        goto NEXT_INSTRUCTION;

    STDOUT_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdout_file);
        goto NEXT_INSTRUCTION;

    STDOUT_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdout_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDIN_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdin_file);
        goto NEXT_INSTRUCTION;

    STDIN_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdin_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDERR_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stderr_file);
        goto NEXT_INSTRUCTION;

    STDERR_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stderr_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDOUT_AND_STDERR_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdout_and_stderr_file);
        goto NEXT_INSTRUCTION;

    STDOUT_AND_STDERR_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdout_and_stderr_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    BACKGROUND_JOB_LABEL:
        toks->data.is_background_job = true;
        goto NEXT_INSTRUCTION;

    ASSIGNMENT_LABEL:
        // skip command line arguments that look like assignment.
        // for example "CC=clang" is an assignment, "make CC=clang" is not.
        if (tok != toks->head->next) {
            tok->op = OP_CONSTANT;
            goto NEXT_INSTRUCTION;
        }

        parser_assignment_process(tok, &shell->vars, &shell->arena);
        if (prev && tok && tok->next) {
            token_set_after(tok->next, prev);
            prev = tok->next;
            if (tok->next->next) {
                tok = tok->next->next;
                continue;
            }
        }
        goto NEXT_INSTRUCTION;

    IF_LABEL:
        result = logic_preprocess(tok, &toks->data, scratch);
        if (result.type == LT_CODE) {
            puts("ncsh: error preprocessing logic, could not process 'if' statement.");
            return result.val.code;
        }

        toks->data.logic_type = result.type;
        tok = result.val.tok;
        toks->head->next = tok;
        goto NEXT_INSTRUCTION;

    FI_LABEL:
        result = logic_preprocess(tok, &toks->data, scratch);
        if (result.type == LT_CODE) {
            puts("ncsh: error preprocessing logic, could not process 'if' statement.");
            return result.val.code;
        }

        toks->data.logic_type = result.type;
        tok = result.val.tok;
        toks->head->next = tok;
        goto NEXT_INSTRUCTION;

    NEXT_INSTRUCTION:
        prev = tok;
        if (tok)
            tok = tok->next;
    }
    ++toks->data.number_of_pipe_commands;

    return EXIT_SUCCESS;
}*/


