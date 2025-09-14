# frozen_string_literal: true

require './acceptance_tests/tests/common'

def basic_ls_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.assert_row_ends_with(row, 'ls')
  @tty.send_return
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
  test_passed('Basic input (ls) test')
  row
end

def basic_bad_command_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('lss') # send a bad command
  @tty.assert_cursor_position(@start_column + 3, row)
  @tty.send_return
  @tty.assert_row_ends_with(row, 'lss')
  row += 1
  @tty.assert_row_starts_with(row, 'ncsh: Could not run command: ')
  row += 1
  test_passed('Bad command test')
  row
end

def basic_tests(row)
  starting_tests('basic')

  row = basic_ls_test(row)
  basic_bad_command_test(row)
end
