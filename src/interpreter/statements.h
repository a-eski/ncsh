#pragma once

#include <unistd.h>
#include <stdint.h>

enum Redirect_Type {
    RT_NONE,
    RT_OUT,
    RT_OUT_APPEND,
    RT_IN,
    RT_IN_APPEND,
    RT_ERR,
    RT_ERR_APPEND,
    RT_OUT_ERR,
    RT_OUT_ERR_APPEND
};

typedef struct {
    size_t count;
    size_t cap;
    enum Ops* ops;
    size_t* lens;
    char** vals;
} Commands;

typedef struct {
    size_t count;
    size_t cap;
    Commands* commands;
} Statement;

typedef struct {
    uint8_t pipes_count;
    enum Redirect_Type redirect_type;
    char* filename;
    bool is_bg_job;

    size_t count;
    size_t cap;
    Statement* statements;
} Statements;
