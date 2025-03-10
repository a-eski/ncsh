# frozen_string_literal: true

require './acceptance_tests/tests/common'

def z_add_tests(row)
  assert_check_new_row(row)
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, %(Added new entry to z database.))
  row += 1
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'Entry already exists in z database.')
  row += 1
  test_passed('z add tests')
  row
end

def z_tests(row)
  starting_tests('z_add')
  z_add_tests(row)
  # row = z_remove_tests(row)
  # row = z_print_tests(row)
end
