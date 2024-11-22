#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'

START_COL = 20
WC_C_LENGTH = '268'
SLEEP_TIME = 0.5

def assert_check_new_row(row)
  @tty.assert_row_starts_with(row, "#{ENV['USER']}->")
  @tty.assert_row_like(row, 'ncsh')
  @tty.assert_row_ends_with(row, '>')
  @tty.assert_cursor_position(START_COL, row)
end

def starting_tests(test)
  puts "===== Starting #{test} tests ====="
end

def basic_startup_time_test(row)
  @tty.assert_row_starts_with(row, 'ncsh: startup time: ')
  row += 1
  puts 'Startup time test passed'
  row
end

def basic_ls_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls))
  @tty.assert_cursor_position(START_COL + 2, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'ls')
  row += 1
  @tty.assert_row_starts_with(row, 'LICENSE')
  row = 9
  puts 'Basic input (ls) test passed'
  row
end

def basic_echo_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(echo hello))
  @tty.assert_cursor_position(START_COL + 10, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'echo hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  puts 'echo hello test passed'
  row
end

def basic_bad_command_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lss)) # send a bad command
  @tty.assert_cursor_position(START_COL + 3, row)
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

  row = basic_startup_time_test row

  row = basic_ls_test row

  row = basic_echo_test row

  basic_bad_command_test row
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
  @tty.assert_row_ends_with(row, 'ls')
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
  @tty.assert_cursor_position(START_COL + 6, row)
  @tty.send_left_arrows(2)
  @tty.assert_cursor_position(START_COL + 4, row)
  @tty.send_backspaces(4)
  @tty.assert_cursor_position(START_COL, row)
  @tty.assert_row_ends_with(row, '> ss')
  @tty.send_right_arrows(2)
  @tty.assert_cursor_position(START_COL + 2, row)
  @tty.send_backspaces(2)
  @tty.assert_cursor_position(START_COL, row)
  @tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
  @tty.send_newline
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
  @tty.assert_cursor_position(START_COL + 1, row)
  @tty.send_left_arrow
  @tty.assert_cursor_position(START_COL, row)
  @tty.send_delete
  assert_check_new_row(row)
  puts 'End of line delete test passed'
  row
end

def midline_delete_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(lssss))
  @tty.assert_cursor_position(START_COL + 5, row)
  @tty.send_left_arrows(4)
  @tty.assert_cursor_position(START_COL + 1, row)
  @tty.send_delete
  @tty.assert_cursor_position(START_COL + 1, row)
  @tty.send_deletes(3)
  @tty.send_left_arrow
  @tty.send_delete
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(echo hello))
  @tty.send_newline
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
  @tty.send_keys_one_at_a_time(%(ls | wc -c))
  @tty.send_newline
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  puts 'Simple pipe test passed'
  row
end

def multiple_pipes_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort | wc -c))
  @tty.send_newline
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

def basic_output_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls > t.txt))
  @tty.assert_row_ends_with(row, %(ls > t.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(head -1 t.txt))
  @tty.assert_row_ends_with(row, %(head -1 t.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  @tty.assert_row_starts_with(row, 'LICENSE')
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(rm t.txt))
  @tty.send_newline
  row += 1
  puts 'Basic output redirection test passed'
  row
end

def piped_output_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort -r > t2.txt))
  @tty.assert_row_ends_with(row, %(ls | sort -r > t2.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(head -1 t2.txt))
  @tty.assert_row_ends_with(row, %(head -1 t2.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  @tty.assert_row_starts_with(row, 'tests_p.sh')
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(rm t2.txt))
  @tty.send_newline
  row += 1
  puts 'Piped output redirection test passed'
  row
end

def multiple_piped_output_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort | wc -c > t3.txt))
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c > t3.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(head -1 t3.txt))
  @tty.assert_row_ends_with(row, %(head -1 t3.txt))
  @tty.send_newline
  sleep SLEEP_TIME
  row += 1
  @tty.assert_row_starts_with(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(rm t3.txt))
  @tty.send_newline
  row += 1
  puts 'Multiple piped output redirection test passed'
  row
end

def output_redirection_tests(row)
  starting_tests 'output redirections'

  row = basic_output_redirection_test row

  row = piped_output_redirection_test row

  multiple_piped_output_redirection_test row
end

def history_tests(row)
  starting_tests 'history'
  row
end

def autocompletion_tests(row)
  starting_tests 'autocompletion'
  row
end

def arrow_tests(row)
  starting_tests 'arrows'
  row
end

def multiline_tests(row)
  starting_tests 'multiline'
  row
end

@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 48)

@row = 0

@row = basic_tests @row

@row = backspace_tests @row

@row = delete_tests @row

@row = pipe_tests @row

@row = output_redirection_tests @row

# puts "\n#{@tty.capture}"

puts 'All tests passed!'
