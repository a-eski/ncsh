# frozen_string_literal: true

def tab_out_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.send_keys(TTYtest::TAB)
  row += 1
  @tty.assert_row_ends_with(row, 'ls > t.txt')
  @tty.send_keys(TTYtest::TAB)
  row += TAB_AUTOCOMPLETE_ROWS
  assert_check_new_row(row)

  test_passed('Tab out of autocompletion')
  row
end

def arrows_move_tab_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.send_keys(TTYtest::TAB)
  row += 1
  cursor_x_before = @tty.cursor_x
  cursor_y_before = @tty.cursor_y
  @tty.send_up_arrow
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before)
  @tty.send_down_arrow
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before + 1)
  @tty.send_down_arrows(TAB_AUTOCOMPLETE_ROWS)
  @tty.assert_cursor_position(cursor_x_before, cursor_y_before + TAB_AUTOCOMPLETE_ROWS - 2)
  @tty.send_keys(TTYtest::TAB)
  row += TAB_AUTOCOMPLETE_ROWS

  test_passed('Arrows autocompletion')
  row
end

def select_tab_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls')
  @tty.send_keys(TTYtest::TAB)
  row += 1
  @tty.send_down_arrows(5)
  @tty.send_newline
  row += TAB_AUTOCOMPLETE_ROWS + 1
  @tty.assert_row_ends_with(row, WC_C_LENGTH)
  row += 1

  test_passed('Select tab autocompletion')
  row
end

def tab_autocompletion_tests(row)
  starting_tests('tab autocompletion')

  row = tab_out_autocompletion_test(row)
  row = arrows_move_tab_autocompletion_test(row)
  select_tab_autocompletion_test(row)
end

def basic_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('l')
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, 'ls > t.txt')
  @tty.send_keys_exact(TTYtest::CTRLU)

  test_passed('Basic autocompletion test')
  row
end

def backspace_and_delete_autocompletion_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time('ls > ')
  @tty.send_backspaces(1)
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, 'ls > t.txt')

  @tty.send_left_arrows(8)
  @tty.send_deletes(8)
  @tty.send_keys_one_at_a_time(' |')
  @tty.send_right_arrow
  @tty.assert_row_ends_with(row, 'ls | sort | wc -c > t3.txt')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, WC_C_LENGTH)
  row += 1

  test_passed('Backspace and delete autocompletion test')
  row
end

def autocompletions_tests(row)
  starting_tests('autocompletion')

  row = basic_autocompletion_test(row)
  backspace_and_delete_autocompletion_test(row)
end

def autocompletion_tests(row)
  row = autocompletions_tests(row)
  tab_autocompletion_tests(row)
end
