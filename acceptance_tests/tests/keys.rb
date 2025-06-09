# frozen_string_literal: true

require './acceptance_tests/tests/common'

def home_and_end_tests(row)
  starting_tests('home and end')

  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ss))
  @tty.send_home
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_end
  @tty.assert_cursor_position(@start_column + 2, row)
  @tty.send_backspaces(2)

  test_passed('Home and End tests')
  row
end

def end_of_line_backspace_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(l))
  @tty.send_backspace
  assert_check_new_row(row)
  test_passed('End of line backspace test')
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
  test_passed('Multiple end of line backspace test')
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
  test_passed('Midline backspace test')
  row
end

def backspace_tests(row)
  starting_tests('backspace')

  row = end_of_line_backspace_test(row)
  row = multiple_end_of_line_backspace_test(row)
  midline_backspace_test(row)
end

def end_of_line_delete_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('s')
  @tty.assert_cursor_position(@start_column + 1, row)
  @tty.send_left_arrow
  @tty.assert_cursor_position(@start_column, row)
  @tty.send_delete
  assert_check_new_row(row)
  test_passed('End of line delete test')
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
  test_passed('Midline delete test')
  row
end

def delete_tests(row)
  starting_tests('delete')

  row = end_of_line_delete_test(row)
  midline_delete_test(row)
end

def delete_line_tests(row)
  starting_tests('delete line')
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys_exact(TTYtest::CTRLU)
  assert_check_new_row(row)
  @tty.send_keys_exact(TTYtest::CTRLU)
  test_passed('Delete line test')
  row
end

def delete_word_tests(row)
  starting_tests('delete word')
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(ls | sort ))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls | sort))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls |))
  @tty.send_keys(TTYtest::CTRLW)
  @tty.assert_row_ends_with(row, %(ls))
  @tty.send_keys(TTYtest::CTRLW)
  test_passed('Delete word test')
  row
end

def keys_tests(row)
  row = home_and_end_tests(row)
  row = backspace_tests(row)
  row = delete_tests(row)
  row = delete_line_tests(row)
  delete_word_tests(row)
end
