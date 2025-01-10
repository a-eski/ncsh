#!/bin/env bash

echo 'before'
wc -l /home/alex/.config/ncsh/.ncsh_history

sort -o /home/alex/.config/ncsh/.ncsh_history -u /home/alex/.config/ncsh/.ncsh_history

echo 'after'
wc -l /home/alex/.config/ncsh/.ncsh_history
