#!/usr/bin/env ruby
# frozen_string_literal: true

# acceptance_tests.rb: the main acceptance tests for ncsh.
# Uses prompt options PROMPT_DIRECTORY_SHORT and PROMPT_SHOW_USER_NORMAL

require 'ttytest'
require './acceptance_tests/expansion'
require './acceptance_tests/autocompletion'
require './acceptance_tests/startup'
require './acceptance_tests/syntax'

WC_C_LENGTH = '141'
SLEEP_TIME = 0.2
LS_LINES = 2
LS_ITEMS = 15
LS_FIRST_ITEM = 'CMakeLists.txt'
TAB_AUTOCOMPLETE_ROWS = 11

def basic_ls_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.assert_row_ends_with(row, 'ls')
  @tty.send_newline
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  test_passed('Basic input (ls) test')
  row
end

def basic_bad_command_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lss)) # send a bad command
  @tty.assert_cursor_position(@start_column + 3, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'lss')
  row += 1
  @tty.assert_row(row, 'ncsh: Could not run command: No such file or directory')
  row += 1
  test_passed('Bad command test')
  row
end

def basic_tests(row)
  starting_tests('basic')

  row = basic_ls_test(row)
  basic_bad_command_test(row)
end

def home_and_end_tests(row)
  starting_tests('home and end')

  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ss))
  @tty.send_home
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_end
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.send_backspaces(2)

  test_passed('Home and End tests')
  row
end

def end_of_line_backspace_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(l))
  @tty.send_backspace
  assert_check_new_row(row)
  test_passed('End of line backspace test')
  row
end

def multiple_end_of_line_backspace_test(row)
  @tty.send_keys_one_at_a_time(%(lsssss))
  @tty.assert_row_ends_with(row, %(lsssss))
  @tty.send_backspaces(4)
  @tty.assert_row_like(row, 'ls') # assert_row_ends_with once autocomplete bugs fixed
  @tty.send_backspaces(2)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
  @tty.send_backspace
  @tty.send_keys_one_at_a_time(%(o))
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('Multiple end of line backspace test')
  row
end

def midline_backspace_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lsssss))
  @tty.assert_cursor_position(@start_column + 6, row)
  @tty.send_left_arrows(2)
  @tty.assert_cursor_position(@start_column + 4, row)
  @tty.send_backspaces(4)
  @tty.assert_cursor_position(@start_column, row)
  @tty.assert_row_ends_with(row, 'ss')
  @tty.send_right_arrows(2)
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.send_backspaces(2)
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_line('echo hello') # make sure buffer is properly formed after backspaces
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('Midline backspace test')
  row
end

def backspace_tests(row)
  starting_tests('backspace')

  row = end_of_line_backspace_test(row)
  row = multiple_end_of_line_backspace_test(row)

  midline_backspace_test(row)
end

def end_of_line_delete_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('s')
  @tty.assert_cursor_position(@start_column + 1, row)
  @tty.send_left_arrow
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_delete
  assert_check_new_row(row)
  test_passed('End of line delete test')
  row
end

def midline_delete_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lssss))
  @tty.assert_cursor_position(@start_column + 5, row)
  @tty.send_left_arrows(4)
  @tty.assert_cursor_position(@start_column + 1, row)
  @tty.send_delete
  @tty.assert_cursor_position(@start_column + 1, row)
  @tty.send_deletes(3)
  @tty.send_left_arrow
  @tty.send_delete
  assert_check_new_row(row)
  @tty.send_line('echo hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('Midline delete test')
  row
end

def delete_tests(row)
  starting_tests('delete')

  row = end_of_line_delete_test(row)
  midline_delete_test(row)
end

def pipe_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('Simple pipe test')
  row
end

def multiple_pipes_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls | sort | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('Multiple pipes test')
  row
end

def pipe_tests(row)
  starting_tests('pipe')

  row = pipe_test(row)
  multiple_pipes_test(row)
end

def basic_history_test(row)
  assert_check_new_row(row)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | sort | wc -c')
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | wc -c')
  @tty.send_down_arrow
  @tty.assert_row_ends_with(row, 'ls | sort | wc -c')
  @tty.send_newline
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('Basic history test')
  row
end

def history_delete_test(row)
  assert_check_new_row(row)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | sort | wc -c')
  @tty.send_left_arrows(12)
  @tty.send_deletes(7)
  @tty.send_newline
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('History delete test')
  row
end

def history_backspace_test(row)
  assert_check_new_row(row)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | wc -c')
  @tty.send_backspaces(5)
  @tty.send_line('head -1')
  @tty.assert_row_ends_with(row, 'ls | head -1')
  row += 1
  @tty.assert_row_ends_with(row, LS_FIRST_ITEM)
  row += 1
  test_passed('History backspace test')
  row
end

def history_clear_test(row)
  assert_check_new_row(row)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | head -1')
  @tty.send_down_arrow
  assert_check_new_row(row)
  test_passed('History clear test')
  row
end

def history_tests(row)
  starting_tests('history')

  row = basic_history_test(row)
  row = history_delete_test(row)
  row = history_backspace_test(row)
  history_clear_test(row)
  # row = history_add_test(row)
  # row = history_remove_test(row)
end

def basic_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t.txt', SLEEP_TIME)
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t.txt')
  row += 1
  test_passed('Basic output redirection test')
  row
end

def piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort -r > t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort -r > t2.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t2.txt))
  row += 1
  @tty.assert_row_starts_with(row, 'tests_z.sh')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t2.txt')
  row += 1
  test_passed('Piped output redirection test')
  row
end

def multiple_piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort | wc -c > t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c > t3.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t3.txt))
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t3.txt')
  row += 1
  test_passed('Multiple piped output redirection test')
  row
end

def stdout_redirection_append_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort | wc -c > t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('ls | sort | wc -c >> t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t3.txt')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.send_line('rm t3.txt')
  row += 1

  test_passed('Stdout redirection append test')
  row
end

def stdout_redirection_tests(row)
  starting_tests('stdout redirection')

  row = basic_stdout_redirection_test(row)
  row = piped_stdout_redirection_test(row)
  row = multiple_piped_stdout_redirection_test(row)
  stdout_redirection_append_test(row)
end

def z_add_tests(row)
  starting_tests('z_add')
  assert_check_new_row(row)
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, %(Added new entry to z database.))
  row += 1
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'Entry already exists in z database.')
  row += 1
  test_passed('z add tests')
  row
end

def basic_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t.txt', SLEEP_TIME)
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('sort < t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_ITEMS
  assert_check_new_row(row)
  @tty.send_line('rm t.txt')
  row += 1
  test_passed('Basic input redirection test')
  row
end

def piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls > t2.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('sort | wc -c < t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(sort | wc -c < t2.txt))
  row += 1
  @tty.assert_row_starts_with(row, (WC_C_LENGTH.to_i + 't2.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t2.txt')
  row += 1
  test_passed('Piped input redirection test')
  row
end

def multiple_piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls > t3.txt))
  row += 1
  @tty.send_line_then_sleep('sort | head -1 | wc -l < t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(sort | head -1 | wc -l < t3.txt))
  row += 1
  @tty.assert_row(row, '1')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t3.txt')
  row += 1
  test_passed('Multiple piped input redirection test')
  row
end

def stdin_redirection_tests(row)
  starting_tests('stdin redirection')

  row = basic_stdin_redirection_test(row)
  row = piped_stdin_redirection_test(row)
  multiple_piped_stdin_redirection_test(row)
end

def help_test(row)
  assert_check_new_row(row)
  @tty.send_line('help')
  row += 1
  @tty.assert_contents_at row, row + 13, <<~TERM
    ncsh Copyright (C) 2025 Alex Eski
    This program comes with ABSOLUTELY NO WARRANTY.
    This is free software, and you are welcome to redistribute it under certain conditions.

    ncsh help:
    Builtin Commands: {command} {args}
    q:                    To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.
    cd/z:                 You can change directory with cd or z.
    echo:                 You can write things to the screen using echo.
    history:              You can see your command history using the history command.
    history count:        You can see the number of entries in your history with history count command.
    pwd:                  Prints the current working directory.
    alex /shells/ncsh ❱
  TERM
  row += 12
  test_passed('Help test')
  row
end

def basic_echo_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(echo hello))
  @tty.assert_cursor_position(@start_column + 10, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'echo hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('basic echo test')
  row
end

def quote_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line("echo 'hello'")
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  @tty.send_line('echo "hello"')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  @tty.send_line('echo `hello`')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  test_passed('quote echo test')
  row
end

def builtin_tests(row)
  starting_tests('builtin')

  # row = help_test(row)
  row = basic_echo_test(row)
  quote_echo_test(row)
end

def delete_line_tests(row)
  starting_tests('delete line')
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys_exact(TTYtest::CTRLU)
  assert_check_new_row(row)
  @tty.send_keys_exact(TTYtest::CTRLU)
  test_passed('Delete line test')
  row
end

def delete_word_tests(row)
  starting_tests('delete word')
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls | sort))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls |))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls))
  @tty.send_keys(TTYtest::CTRLW)
  test_passed('Delete word test')
  row
end

def run_acceptance_tests(prompt_directory_option, prompt_user_option, is_custom_prompt)
  setup_tests(prompt_directory_option, prompt_user_option, is_custom_prompt)
  row = 0
  @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 120, height: 140)

  row = startup_tests(row, true)
  row = basic_tests(row)
  row = home_and_end_tests(row)
  row = backspace_tests(row)
  row = delete_tests(row)
  row = pipe_tests(row)
  row = history_tests(row)
  row = stdout_redirection_tests(row)
  row = z_add_tests(row)
  # row = z_remove_tests(row)
  # row = z_print_tests(row)
  row = stdin_redirection_tests(row)
  row = builtin_tests(row)
  row = delete_line_tests(row)
  delete_word_tests(row)
  @tty.send_keys_exact(%(quit))
  @tty.send_newline

  row = 0
  @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 180, height: 150)
  row = startup_tests(row, false)
  row = autocompletion_tests(row)
  row = syntax_tests(row)
  # row = paste_tests(row)
  # row = multiline_tests(row)
  expansion_tests(row)
end
