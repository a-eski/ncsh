# frozen_string_literal: true

require './acceptance_tests/tests/common'

def assert_history_up_result(row, command)
  @tty.send_up_arrow
  @tty.assert_row_ends_with(row, command)
end

def assert_history_down_result(row, command)
  @tty.send_down_arrow
  @tty.assert_row_ends_with(row, command)
end

def basic_history_test(row)
  assert_check_new_row(row)
  assert_history_up_result(row, 'ls | sort | wc -c')
  assert_history_up_result(row, 'ls | wc -c')
  assert_history_down_result(row, 'ls | sort | wc -c')
  @tty.send_newline
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('Basic history test')
  row
end

def history_delete_test(row)
  assert_check_new_row(row)
  assert_history_up_result(row, 'ls | sort | wc -c')
  @tty.send_left_arrows(12)
  @tty.send_deletes(7)
  @tty.send_newline
  row += 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1
  test_passed('History delete test')
  row
end

def history_backspace_test(row)
  assert_check_new_row(row)
  assert_history_up_result(row, 'ls | wc -c')
  @tty.send_backspaces(5)
  @tty.send_line('head -1')
  @tty.assert_row_ends_with(row, 'ls | head -1')
  row += 1
  @tty.assert_row_ends_with(row, LS_FIRST_ITEM)
  row += 1
  test_passed('History backspace test')
  row
end

def history_clear_test(row)
  assert_check_new_row(row)
  assert_history_up_result(row, 'ls | head -1')
  @tty.send_down_arrow
  assert_check_new_row(row)
  test_passed('History clear test')
  row
end

def history_tests(row)
  starting_tests('history')

  row = basic_history_test(row)
  row = history_delete_test(row)
  row = history_backspace_test(row)
  history_clear_test(row)
  # row = history_add_test(row)
  # row = history_remove_test(row)
  # row = history_clean_test(row)
end
