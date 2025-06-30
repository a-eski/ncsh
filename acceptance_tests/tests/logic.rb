# frozen_string_literal: true

require './acceptance_tests/tests/common'

def and_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls && ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  test_passed('and (&&) test')
  row
end

def multiple_and_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls && ls && ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES
  test_passed('multiple and (&&) test')
  row
end

def true_and_test(row)
  test_passed('true and (&&) test')
  row
end

def false_and_test(row)
  assert_check_new_row(row)
  @tty.send_line('false && ls')
  row += 1
  test_passed('false and (&&) test')
  row
end

def and_tests(row)
  starting_tests('and')
  row = and_test(row)
  row = multiple_and_test(row)
  row = true_and_test(row)
  false_and_test(row)
end

def or_test(row)
  assert_check_new_row(row)
  test_passed('or (||) test')
  row
end

def multiple_or_test(row)
  assert_check_new_row(row)
  test_passed('multiple or (||) test')
  row
end

def true_or_test(row)
  assert_check_new_row(row)
  @tty.send_line('true || ls')
  row += 1
  assert_check_new_row(row)
  test_passed('true or (||) test')
  row
end

def false_or_test(row)
  assert_check_new_row(row)
  @tty.send_line('true || ls')
  row += 1
  assert_check_new_row(row)
  test_passed('false or (||) test')
  row
end

def or_tests(row)
  starting_tests('or')
  row = or_test(row)
  row = multiple_or_test(row)
  row = true_or_test(row)
  false_or_test(row)
end

def logic_tests(row)
  row = and_tests(row)
  or_tests(row)
end
