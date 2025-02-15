#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'

WC_C_LENGTH = '141'
SLEEP_TIME = 0.2
LS_LINES = 2
LS_ITEMS = 15
LS_FIRST_ITEM = 'CMakeLists.txt'
TAB_AUTOCOMPLETE_ROWS = 11

def setup_tests
  @user = ENV['USER']
  @start_column = 16 + @user.length
end

def assert_check_new_row(row)
  @tty.assert_row_starts_with(row, "#{@user} ")
  @tty.assert_row_like(row, 'ncsh')
  @tty.assert_row_ends_with(row, ' ❱ ')
  @tty.assert_cursor_position(@start_column, row)
end

def starting_tests(test)
  puts "===== Starting #{test} tests ====="
end

def z_database_new_test(row)
  @tty.assert_row(row, 'Error opening z database file: No such file or directory')
  row += 1
  @tty.assert_row(row, 'Trying to create z database file.')
  row += 1
  @tty.assert_row(row, 'Created z database file.')
  row += 1
  puts 'New z database test passed'
  row
end

def startup_test(row)
  @tty.assert_row_starts_with(row, 'ncsh: startup time: ')
  row += 1
  puts 'Startup time test passed'
  row
end

def newline_sanity_test(row)
  assert_check_new_row(row)
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  @tty.send_newline
  row += 1
  puts 'Newline sanity test passed'
  row
end

def empty_line_arrow_check(row)
  @tty.send_left_arrow
  assert_check_new_row(row)
  @tty.send_right_arrow
  assert_check_new_row(row)
end

def empty_line_sanity_test(row)
  assert_check_new_row(row)
  empty_line_arrow_check(row)
  @tty.send_backspace
  assert_check_new_row(row)
  @tty.send_delete
  assert_check_new_row(row)
  puts 'Empty line sanity test passed'
  row
end

def startup_tests(row, run_z_database_new_tests)
  starting_tests 'startup tests'

  row = z_database_new_test row if run_z_database_new_tests
  row = startup_test row
  row = newline_sanity_test row

  empty_line_sanity_test row
end

def basic_ls_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.assert_row_ends_with(row, 'ls')
  @tty.send_newline
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  puts 'Basic input (ls) test passed'
  row
end

def basic_bad_command_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lss)) # send a bad command
  @tty.assert_cursor_position(@start_column + 3, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'lss')
  row += 1
  @tty.assert_row(row, 'ncsh: Could not find command or directory: No such file or directory')
  row += 1
  puts 'Bad command test passed'
  row
end

def basic_tests(row)
  starting_tests 'basic'

  row = basic_ls_test row
  basic_bad_command_test row
end

def home_and_end_tests(row)
  starting_tests 'home and end'

  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ss))
  @tty.send_home
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_end
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.send_backspaces(2)

  puts 'Home and End tests passed'
  row
end

def end_of_line_backspace_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(l))
  @tty.send_backspace
  assert_check_new_row(row)
  puts 'End of line backspace test passed'
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
  puts 'Multiple end of line backspace test passed'
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
  puts 'Midline backspace test passed'
  row
end

def backspace_tests(row)
  starting_tests 'backspace'

  row = end_of_line_backspace_test row
  row = multiple_end_of_line_backspace_test row

  midline_backspace_test row
end

def end_of_line_delete_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('s')
  @tty.assert_cursor_position(@start_column + 1, row)
  @tty.send_left_arrow
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_delete
  assert_check_new_row(row)
  puts 'End of line delete test passed'
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
  puts 'Midline delete test passed'
  row
end

def delete_tests(row)
  starting_tests 'delete'

  row = end_of_line_delete_test row
  midline_delete_test row
end

def pipe_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  puts 'Simple pipe test passed'
  row
end

def multiple_pipes_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls | sort | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  puts 'Multiple pipes test passed'
  row
end

def pipe_tests(row)
  starting_tests 'pipe'

  row = pipe_test row
  multiple_pipes_test row
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
  puts 'Basic history test passed'
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
  puts 'History delete test passed'
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
  puts 'History backspace test passed'
  row
end

def history_clear_test(row)
  assert_check_new_row(row)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, 'ls | head -1')
  @tty.send_down_arrow
  assert_check_new_row(row)
  puts 'History clear test passed'
  row
end

def history_tests(row)
  starting_tests 'history'

  row = basic_history_test row
  row = history_delete_test row
  row = history_backspace_test row
  history_clear_test row
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
  puts 'Basic output redirection test passed'
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
  puts 'Piped output redirection test passed'
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
  puts 'Multiple piped output redirection test passed'
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

  puts 'Stdout redirection append test passed'
  row
end

def stdout_redirection_tests(row)
  starting_tests 'stdout redirection'

  row = basic_stdout_redirection_test row
  row = piped_stdout_redirection_test row
  row = multiple_piped_stdout_redirection_test row
  stdout_redirection_append_test row
end

def z_add_tests(row)
  starting_tests 'z_add'
  assert_check_new_row(row)
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, %(Added new entry to z database.))
  row += 1
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'Entry already exists in z database.')
  row += 1
  puts 'z add tests passed'
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
  puts 'Basic input redirection test passed'
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
  puts 'Piped input redirection test passed'
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
  puts 'Multiple piped input redirection test passed'
  row
end

def stdin_redirection_tests(row)
  starting_tests 'stdin redirection'

  row = basic_stdin_redirection_test row
  row = piped_stdin_redirection_test row
  multiple_piped_stdin_redirection_test row
end

def basic_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(l))
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c))
  @tty.send_keys_exact(TTYtest::CTRLU)

  puts 'Basic autocompletion test passed'
  row
end

def backspace_and_delete_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls > ))
  @tty.send_backspaces(1)
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, %(ls > t.txt))

  @tty.send_left_arrows(8)
  @tty.send_deletes(8)
  @tty.send_keys_one_at_a_time(%( |))
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c > t3.txt))
  @tty.send_newline
  row += 1
  @tty.assert_row(row, WC_C_LENGTH)
  row += 1

  puts 'Backspace and delete autocompletion test passed'
  row
end

def autocompletion_tests(row)
  starting_tests 'autocompletion'

  row = basic_autocompletion_test row
  backspace_and_delete_autocompletion_test row
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
  puts 'Help test passed'
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
  puts 'basic echo test passed'
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

  puts 'quote echo test passed'
  row
end

def builtin_tests(row)
  starting_tests 'builtin'

  # row = help_test row
  row = basic_echo_test row
  quote_echo_test row
end

def delete_line_tests(row)
  starting_tests 'delete line'
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys_exact(TTYtest::CTRLU)
  assert_check_new_row(row)
  @tty.send_keys_exact(TTYtest::CTRLU)
  puts 'Delete line test passed'
  row
end

def delete_word_tests(row)
  starting_tests 'delete word'
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls | sort))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls |))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls))
  @tty.send_keys(TTYtest::CTRLW)
  puts 'Delete word test passed'
  row
end

def tab_out_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls))
  @tty.send_keys(TTYtest::TAB)
  row += 1
  @tty.assert_row_ends_with(row, %(ls > t.txt))
  @tty.send_keys(TTYtest::TAB)
  row += TAB_AUTOCOMPLETE_ROWS
  assert_check_new_row(row)

  puts 'Tab out of autocompletion test passed'
  row
end

def arrows_move_tab_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls))
  @tty.send_keys(TTYtest::TAB)
  row += 1
  cursor_x_before = @tty.cursor_x
  cursor_y_before = @tty.cursor_y
  @tty.send_up_arrow
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before)
  @tty.send_down_arrow
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before + 1)
  @tty.send_down_arrows(TAB_AUTOCOMPLETE_ROWS)
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before + TAB_AUTOCOMPLETE_ROWS - 2)
  @tty.send_keys(TTYtest::TAB)
  row += TAB_AUTOCOMPLETE_ROWS

  puts 'Arrows autocompletion test passed'
  row
end

def select_tab_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls))
  @tty.send_keys(TTYtest::TAB)
  row += 1
  @tty.send_down_arrows(5)
  @tty.send_newline
  row += TAB_AUTOCOMPLETE_ROWS + 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1

  puts 'Select tab autocompletion test passed'
  row
end

def tab_autocompletion_tests(row)
  starting_tests 'tab autocompletion'

  row = tab_out_autocompletion_test row
  row = arrows_move_tab_autocompletion_test row
  # select_tab_autocompletion_test row
end

def assert_check_syntax_error(row, input)
  assert_check_new_row(row)
  @tty.send_line(input)
  row += 1
  @tty.assert_row_starts_with(row, 'ncsh: Invalid syntax:')
  row += 1
  row
end

def operators_invalid_syntax_first_position_test(row)
  # tries sending operator as only character to ensure invalid syntax is shown to user
  row = assert_check_syntax_error(row, '|') # pipe
  row = assert_check_syntax_error(row, '>') # output redirection
  row = assert_check_syntax_error(row, '>>') # output redirection append
  row = assert_check_syntax_error(row, '<') # input redirection
  row = assert_check_syntax_error(row, '2>') # error redirection
  row = assert_check_syntax_error(row, '2>>') # error redirection append
  row = assert_check_syntax_error(row, '&>') # output and error redirection
  row = assert_check_syntax_error(row, '&>>') # output and error redirection append
  row = assert_check_syntax_error(row, '&') # background job
  puts 'Invalid syntax in first position test passed'
  row
end

def operators_invalid_syntax_last_position_test(row)
  row = assert_check_syntax_error(row, 'ls |') # pipe
  row = assert_check_syntax_error(row, 'ls >') # output redirection
  row = assert_check_syntax_error(row, 'ls >>') # output redirection append
  row = assert_check_syntax_error(row, 'sort <') # input redirection
  row = assert_check_syntax_error(row, 'ls 2>') # error redirection
  row = assert_check_syntax_error(row, 'ls 2>>') # error redirection append
  row = assert_check_syntax_error(row, 'ls &>') # output and error redirection
  row = assert_check_syntax_error(row, 'ls &>>') # output and error redirection append
  puts 'Invalid syntax in last position test passed'
  row
end

# invalid operator usage to ensure invalid syntax is shown to user
def syntax_error_tests(row)
  starting_tests 'syntax errors'

  row = operators_invalid_syntax_first_position_test row
  operators_invalid_syntax_last_position_test row
end

def basic_stderr_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('lss 2> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t4.txt')
  row += 1
  @tty.assert_row_ends_with(row, 'No such file or directory')
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stderr redirection test passed'
  row
end

def stderr_redirection_append_test(row)
  assert_check_new_row(row)

  puts 'Stderr redirection append test passed'
  row
end

def stderr_redirection_tests(row)
  starting_tests 'sterr redirection'

  row = basic_stderr_redirection_test row
  stderr_redirection_append_test row
end

def basic_stdout_and_stderr_redirection_stderr_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('lss &> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t4.txt')
  row += 1
  @tty.assert_row_ends_with(row, 'No such file or directory')
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stdout and stderr redirection stderr test passed'
  row
end

def basic_stdout_and_stderr_redirection_stdout_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls &> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('cat t4.txt | head -1', SLEEP_TIME)
  row += 1
  @tty.assert_row_ends_with(row, LS_FIRST_ITEM)
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stdout and stderr redirection stdout test passed'
  row
end

def stdout_and_stderr_redirection_tests(row)
  starting_tests 'stdout and sterr redirection'

  row = basic_stdout_and_stderr_redirection_stderr_test row
  basic_stdout_and_stderr_redirection_stdout_test row
end

row = 0
@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 120, height: 140)
setup_tests()

row = startup_tests(row, true)
row = basic_tests row
row = home_and_end_tests row
row = backspace_tests row
row = delete_tests row
row = pipe_tests row
row = history_tests row
row = stdout_redirection_tests row
row = z_add_tests row
row = stdin_redirection_tests row
row = builtin_tests row
row = delete_line_tests row
row = delete_word_tests row
@tty.send_keys_exact(%(quit))
@tty.send_newline

row = 0
@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 180, height: 150)
row = startup_tests(row, false)
row = autocompletion_tests row
row = tab_autocompletion_tests row
row = syntax_error_tests row
row = stderr_redirection_tests row
row = stdout_and_stderr_redirection_tests row

# row = 0
# @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 20, height: 5)
# row =
# multiline_tests row

# row = home_expansion_tests row
# row = star_expansion_tests row
# row = question_expansion_tests row
# row = copy_paste_tests row

# troubleshooting
# @tty.print
# @tty.print_rows
# p @tty.to_s

puts 'All tests passed!'
