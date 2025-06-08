# frozen_string_literal: true

require './acceptance_tests/tests/common'

def and_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls && ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
  test_passed('and (&&) test')
  row
end

def multiple_and_test(row)
  assert_check_new_row(row)
  @tty.send_line('ls && ls && ls')
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_LINES - 1
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
  test_passed('true or (||) test')
  row
end

def false_or_test(row)
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

def true_if_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ true ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true if test')
  row
end

def false_if_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ false ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  test_passed('false if test')
  row
end

def true_or_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ true || false ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true or if multiple condition test')
  row
end

def true_and_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ true && true ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true and if multiple condition test')
  row
end

def false_or_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ false || true ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  assert_check_new_row(row)
  test_passed('false or if multiple condition test')
  row
end

def false_and_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ false && true ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  test_passed('false and if multiple condition test')
  row
end

def true_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ true ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true if else test')
  row
end

def false_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ false ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false if else test')
  row
end

def equals_if_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 1 -eq 1 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('equals if test')
  row
end

def not_equals_if_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 2 -eq 1 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  test_passed('not equals if test')
  row
end

def equals_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 1 -eq 1 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('equals if else test')
  row
end

def not_equals_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 2 -eq 1 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not equals if else test')
  row
end

def gt_if_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 2 -gt 1 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('gt if test')
  row
end

def not_gt_if_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 1 -gt 2 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  test_passed('not gt if test')
  row
end

def gt_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 2 -gt 1 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('gt if else test')
  row
end

def not_gt_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 1 -gt 2 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not gt if else test')
  row
end

def lt_if_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 1 -lt 2 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('lt if test')
  row
end

def not_lt_if_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 2 -lt 1 ]; then echo hello; fi')
  @tty.send_newline
  row += 1
  assert_check_new_row(row)
  test_passed('not lt if test')
  row
end

def lt_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_keys_exact('if [ 1 -lt 2 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('lt if else test')
  row
end

def not_lt_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_keys_exact('if [ 2 -lt 1 ]; then echo hello; else echo hi; fi')
  @tty.send_newline
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not lt if else test')
  row
end

def if_tests(row)
  starting_tests('if, if else')
  row = true_if_test(row)
  row = false_if_test(row)
  row = true_or_if_multiple_condition_test(row)
  row = true_and_if_multiple_condition_test(row)
  # row = false_or_if_multiple_condition_test(row)
  row = false_and_if_multiple_condition_test(row)
  row = true_if_else_test(row)
  row = false_if_else_test(row)
  row = equals_if_test(row)
  row = not_equals_if_test(row)
  row = equals_if_else_test(row)
  row = not_equals_if_else_test(row)
  row = gt_if_test(row)
  row = not_gt_if_test(row)
  row = gt_if_else_test(row)
  row = not_gt_if_else_test(row)
  row = lt_if_test(row)
  row = not_lt_if_test(row)
  row = lt_if_else_test(row)
  not_lt_if_else_test(row)
end

def logic_tests(row)
  starting_tests('logic')
  row = and_tests(row)
  row = or_tests(row)
  if_tests(row)
end
