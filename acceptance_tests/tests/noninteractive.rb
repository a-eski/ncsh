# frozen_string_literal: true

require 'ttytest'
require './acceptance_tests/tests/common'

WC_C_LENGTH_NO_Z = (WC_C_LENGTH.to_i - '_z_database.bin'.length - 1).to_s

# single command like ls
def single_command_test(row)
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh ls')
  @tty.assert_row_ends_with(row, 'ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1

  test_passed('single command test')
  row
end

# bad command
def bad_command_test(row)
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh lss')
  @tty.assert_row_ends_with(row, 'lss')
  row += 1
  @tty.assert_row(row, 'ncsh: Could not run command: No such file or directory')
  row += 1
  test_passed('bad command test')
  row
end

def basic_tests(row)
  starting_tests('basic noninteractive')
  row = single_command_test(row)
  bad_command_test(row)
end

# pipe
def pipe_command_test(row)
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh ls | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH_NO_Z)
  row += 1
  test_passed('pipe command test')
  row
end

# multiple pipes
def multiple_pipes_command_test(row)
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh ls | sort | wc -c')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH_NO_Z)
  row += 1
  test_passed('multiple pipes command test')
  row
end

def pipe_tests(row)
  starting_tests('pipes noninteractive')
  row = pipe_command_test(row)
  multiple_pipes_command_test(row)
end

# stdout redirection
def basic_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh ls > t.txt', SLEEP_TIME)
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh head -1 t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh rm t.txt')
  row += 1
  test_passed('Basic output redirection test')
  row
end

# piped stdout redirection
def piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh ls | sort -r > t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort -r > t2.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh head -1 t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t2.txt))
  row += 1
  @tty.assert_row_starts_with(row, 'tests_z.sh')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh rm t2.txt')
  row += 1
  test_passed('Piped output redirection test')
  row
end

# multiple pipes stdout redirection
def multiple_piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh ls | sort | wc -c > t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c > t3.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh head -1 t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t3.txt))
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_Z.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('./bin/ncsh rm t3.txt')
  row += 1
  test_passed('Multiple piped output redirection test')
  row
end

def stdout_redirection_append_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('./bin/ncsh ls | sort | wc -c > t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('./bin/ncsh ls | sort | wc -c >> t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('./bin/ncsh cat t3.txt')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_Z.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_Z.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.send_line_then_sleep('./bin/ncsh rm t3.txt', SLEEP_TIME)
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

# stdout append redirection
# piped stdout append redirection

# stdin redirection
# piped stdin redirection
# multiple pipes stdin redirection

# builtins

# syntax

# expansion

def assert_check_new_row(row)
  @tty.assert_row(row, '$')
end

def run_noninteractive_acceptance_tests
  row = 0
  @tty = TTYtest.new_terminal(%(PS1='$ ' /bin/sh), width: 180, height: 80)
  row = basic_tests(row)
  row = pipe_tests(row)
  row = stdout_redirection_tests(row)
end
