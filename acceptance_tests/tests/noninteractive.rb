# frozen_string_literal: true

require 'ttytest'
require './acceptance_tests/tests/common'
require './acceptance_tests/tests/variables'

# single command like ls
def single_command_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh ls')
  @tty.assert_row_ends_with(row, 'ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 2

  test_passed('single command test')
  row
end

# bad command
def bad_command_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh lss')
  @tty.assert_row_ends_with(row, 'lss')
  row += 1
  @tty.assert_row_starts_with(row, 'ncsh: Could not run command: ')
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
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "ls | wc -c"')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH_NO_LOCAL_FILES)
  row += 1
  test_passed('pipe command test')
  row
end

# multiple pipes
def multiple_pipes_command_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "ls | sort | wc -c"')
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH_NO_LOCAL_FILES)
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
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls > t.txt"', SLEEP_TIME)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "head -1 t.txt"', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t.txt"')
  row += 1
  test_passed('Basic output redirection test')
  row
end

# piped stdout redirection
def piped_stdout_redirection_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls | sort -r > t2.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, '"ls | sort -r > t2.txt"')
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "head -1 t2.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, '"head -1 t2.txt"')
  row += 1
  @tty.assert_row_starts_with(row, HEAD_ONE_ITEM)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t2.txt"')
  row += 1
  test_passed('Piped output redirection test')
  row
end

# multiple pipes stdout redirection
def multiple_piped_stdout_redirection_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls | sort | wc -c > t3.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, '"ls | sort | wc -c > t3.txt"')
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "head -1 t3.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, '"head -1 t3.txt"')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_LOCAL_FILES.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t3.txt"')
  row += 1
  test_passed('Multiple piped output redirection test')
  row
end

# stdout append redirection
def stdout_redirection_append_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls | sort | wc -c > t3.txt"', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('./bin/ncsh "ls | sort | wc -c >> t3.txt"', SLEEP_TIME)
  row += 1
  @tty.send_line('./bin/ncsh "cat t3.txt"')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_LOCAL_FILES.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_LOCAL_FILES.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.send_line_then_sleep('./bin/ncsh "rm t3.txt"', SLEEP_TIME)
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

# piped stdout append redirection

# stdin redirection
def basic_stdin_redirection_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls > t.txt"', SLEEP_TIME)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "sort < t.txt"', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_ITEMS_NO_Z + 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t.txt"')
  row += 1
  test_passed('Basic input redirection test')
  row
end

# piped stdin redirection
def piped_stdin_redirection_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls > t2.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, './bin/ncsh "ls > t2.txt"')
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "sort | wc -c < t2.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, './bin/ncsh "sort | wc -c < t2.txt"')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH_NO_LOCAL_FILES.to_i + 't2.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t2.txt"')
  row += 1
  test_passed('Piped input redirection test')
  row
end

# multiple pipes stdin redirection
def multiple_piped_stdin_redirection_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line_then_sleep('./bin/ncsh "ls > t3.txt"', SLEEP_TIME)
  @tty.assert_row_ends_with(row, './bin/ncsh "ls > t3.txt"')
  row += 1
  @tty.send_line_then_sleep("./bin/ncsh 'sort | head -1 | wc -l < t3.txt'", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "./bin/ncsh 'sort | head -1 | wc -l < t3.txt'")
  row += 1
  @tty.assert_row(row, '13')
  row += 1
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "rm t3.txt"')
  row += 1
  test_passed('Multiple piped input redirection test')
  row
end

def stdin_redirection_tests(row)
  starting_tests('stdin redirection')

  # row =
  basic_stdin_redirection_test(row)
  # row = piped_stdin_redirection_test(row)
  # multiple_piped_stdin_redirection_test(row)
end

# expansion
def home_expansion_echo_test(row)
  assert_check_new_row_noninteractive(row)
  @tty.send_line('./bin/ncsh "echo ~"')
  row += 1
  home = ENV['HOME']
  @tty.assert_row(row, home)
  row += 1
  test_passed('home expansion echo test')
  row
end

def home_expansion_tests(row)
  starting_tests('home expansion')
  home_expansion_echo_test(row)
end

def ls_star_expansion_test(row)
  @tty.send_line('./bin/ncsh "ls *.md"')
  row += 1
  @tty.assert_row(row, 'COMPILE.md  NOTES.md  README.md')
  row += 1
  test_passed('ls star expansion test')
  row
end

def star_expansion_tests(row)
  starting_tests('star expansion')
  ls_star_expansion_test(row)
end

def ls_question_expansion_test(row)
  @tty.send_line('./bin/ncsh "ls src/z/?.c"')
  row += 1
  @tty.assert_row(row, 'src/z/z.c')
  row += 1
  test_passed('ls star expansion test')
  row
end

def question_expansion_tests(row)
  starting_tests('question expansion')
  ls_question_expansion_test(row)
end

def expansion_tests(row)
  row = home_expansion_tests(row)
  row = star_expansion_tests(row)
  question_expansion_tests(row)
end

# assertions
def assert_check_new_row_noninteractive(row)
  @tty.assert_row_ends_with(row, '$')
end

# run tests
def run_noninteractive_acceptance_tests
  row = 0
  @tty = TTYtest.new_terminal("PS1='$ ' /bin/sh", width: 180, height: 160)
  row = basic_tests(row)
  row = pipe_tests(row)
  row = stdout_redirection_tests(row)
  row = stdin_redirection_tests(row)
  # row =
  expansion_tests(row)
  # variables_tests(row) # ?
end
