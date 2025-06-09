# frozen_string_literal: true

require './acceptance_tests/tests/common'

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
