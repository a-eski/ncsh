# frozen_string_literal: true

require 'ttytest'
require './acceptance_tests/startup'

def home_expansion_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo ~')
  home = ENV['HOME']
  row += 1
  @tty.assert_row(row, home)
  test_passed('home expansion echo test')
  row
end

def home_expansion_tests(row)
  starting_tests('home expansion')
  home_expansion_echo_test(row)
end

def star_expansion_tests(row)
  starting_tests('star expansion')
  row
end

def expansion_tests(row)
  home_expansion_tests(row)
  # row = star_expansion_tests(row)
  # row = question_expansion_tests(row)
end
