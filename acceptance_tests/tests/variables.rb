# frozen_string_literal: true

require './acceptance_tests/tests/common'

def basic_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('STR=hello')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('echo $STR')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('basic variables test')
  row
end

def quoted_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('STR2="world"')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('echo $STR2')
  row += 1
  @tty.assert_row(row, 'world')
  row += 1
  test_passed('quoted variables test')
  row
end

def command_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('command=ls')
  @tty.send_line('$command')
  row += 2
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES_NO_Z
  test_passed('command variables test')
  row
end

# variables currently don't work when commands are separated like this
def combined_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('wcc="wc -c"')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('ls | $wcc')
  @tty.assert_row(row, WC_C_LENGTH)
  row += 1
  test_passed('combined variables test')
  row
end

def multiple_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('STR3=`hi,`')
  @tty.send_line('STR4=`you!`')
  row += 2
  assert_check_new_row(row)
  @tty.send_line('echo $STR3 $STR4')
  row += 1
  @tty.assert_row(row, 'hi, you!')
  test_passed('multiple variables test')
  row
end

# def variable_execution_test(row)
#   @tty.send_line('echo $(ls)')
#   test_passed('multiple variables test')
#   row
# end

def variables_tests(row)
  starting_tests('variables')
  row = basic_variables_test(row)
  row = quoted_variables_test(row)
  row = command_variables_test(row)
  # row = combined_variables_test(row)
  multiple_variables_test(row)
end
