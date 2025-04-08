#!/bin/env bash

create_dir_if_not_exists() {
    if [ ! -d "$1" ]; then
        mkdir "$1"
    fi
}

create_dir_if_not_exists "Z_CORPUS"
create_dir_if_not_exists "Z_ADD_CORPUS"
create_dir_if_not_exists "NCSH_AUTOCOMPLETIONS_CORPUS"
create_dir_if_not_exists "NCSH_HISTORY_CORPUS"
create_dir_if_not_exists "NCSH_PARSER_CORPUS"
