# frozen_string_literal: true

require './acceptance_tests/tests/common'

def if_variable_test(row)
  assert_check_new_row(row)
  @tty.send_line('VAL=1')
  row += 1
  @tty.send_line_exact('if [ $VAL -eq 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('if variable test')
  row
end

def if_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('VAL2=2')
  row += 1
  @tty.send_line_exact('if [ $VAL -eq $VAL2 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  test_passed('if variables test')
  row
end

def if_variable_tests(row)
  starting_tests('if variable')
  row = if_variable_test(row)
  if_variables_test(row)
end

def if_tests(row)
  if_variable_tests(row)
end
