# frozen_string_literal: true

require './acceptance_tests/tests/common'

# Tests
def version_test(row)
  @tty.assert_row_starts_with(row, 'ncsh version: ')
  row += 1
  test_passed('version test')
  row
end

def z_database_new_test(row)
  @tty.assert_row(row, 'ncsh z: z database file could not be found or opened: No such file or directory')
  row += 1
  @tty.assert_row(row, 'ncsh z: trying to create z database file.')
  row += 1
  @tty.assert_row(row, 'ncsh z: created z database file.')
  row += 1
  test_passed('New z database test')
  row
end

def startup_test(row)
  @tty.assert_row_starts_with(row, 'ncsh startup time: ')
  row += 1
  test_passed('Startup time test')
  row
end

def newline_sanity_test(row)
  assert_check_new_row(row)
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  @tty.send_newline
  row += 1
  test_passed('Newline sanity test')
  row
end

def empty_line_arrow_check(row)
  @tty.send_left_arrow
  assert_check_new_row(row)
  @tty.send_right_arrow
  assert_check_new_row(row)
end

def empty_line_sanity_test(row)
  assert_check_new_row(row)
  empty_line_arrow_check(row)
  @tty.send_backspace
  assert_check_new_row(row)
  @tty.send_delete
  assert_check_new_row(row)
  test_passed('Empty line sanity test')
  row
end

def startup_tests(row, run_z_database_new_tests)
  starting_tests('startup')

  row = version_test(row)
  row = z_database_new_test(row) if run_z_database_new_tests
  row = startup_test(row)
  row = newline_sanity_test(row)

  empty_line_sanity_test(row)
end
