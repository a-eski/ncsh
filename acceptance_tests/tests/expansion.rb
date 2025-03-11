# frozen_string_literal: true

require 'ttytest'
require './acceptance_tests/tests/common'

def home_expansion_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo ~')
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
  @tty.send_line('ls *.md')
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
  @tty.send_line('ls src/z/?.c')
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
