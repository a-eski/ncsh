# frozen_string_literal: true

require './acceptance_tests/tests/common'

# def z_database_new_test(row) is located in startup.rb

def z_add_entry_checked(row)
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, %(z: Added new entry to z database.))
  row += 1
  row
end

def z_add_tests(row)
  assert_check_new_row(row)
  row = z_add_entry_checked(row)
  @tty.send_line('z add ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'z: Entry already exists in z database.')
  row += 1
  test_passed('z add tests')
  row
end

def z_remove_test(row)
  assert_check_new_row(row)
  row = z_add_entry_checked(row)
  @tty.send_line('z remove ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'z: Removed entry from z database.')
  row += 1
  test_passed('z remove test')
  row
end

# rm is alias for remove, both should work the same way
def z_rm_test(row)
  assert_check_new_row(row)
  row = z_add_entry_checked(row)
  @tty.send_line('z rm ~/.config')
  row += 1
  @tty.assert_row_ends_with(row, 'z: Removed entry from z database.')
  row += 1
  test_passed('z rm test')
  row
end

def z_remove_tests(row)
  row = z_remove_test(row)
  row = z_rm_test(row)
  test_passed('z remove tests')
  row
end

def z_tests(row)
  starting_tests('z')
  row = z_remove_tests(row)
  z_add_tests(row)
  # z_print_tests(row)
end
