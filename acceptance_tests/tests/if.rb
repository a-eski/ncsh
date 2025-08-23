# frozen_string_literal: true

require './acceptance_tests/tests/common'

# bool if tests
def true_if_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true if test')
  row
end

def false_if_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('false if test')
  row
end

def true_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true if else test')
  row
end

def false_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false if else test')
  row
end

def true_and_true_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true && true ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true and if multiple condition test')
  row
end

def true_and_false_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true && false ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('true and if multiple condition test')
  row
end

def false_and_true_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && true ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and if multiple condition test')
  row
end

def false_and_false_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && false ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and if multiple condition test')
  row
end

def true_or_true_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true || true ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true or if multiple condition test')
  row
end

def true_or_false_if_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true || false ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true or if multiple condition test')
  row
end

def false_or_true_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false || true ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  assert_check_new_row(row)
  test_passed('false or if multiple condition test')
  row
end

def false_or_false_if_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false || false ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('false or if multiple condition test')
  row
end

def true_and_true_if_else_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true && true ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true and if multiple condition test')
  row
end

def true_and_false_if_else_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true && false ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  test_passed('true and if multiple condition test')
  row
end

def false_and_true_if_else_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && true ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and if multiple condition test')
  row
end

def false_and_false_if_else_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && false ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and if multiple condition test')
  row
end

def true_or_true_if_else_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true || true ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true or if multiple condition test')
  row
end

def true_or_false_if_else_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true || false ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('true or if multiple condition test')
  row
end

def false_or_true_if_else_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false || true ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  assert_check_new_row(row)
  test_passed('false or if multiple condition test')
  row
end

def false_or_false_if_else_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false || false ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false or if multiple condition test')
  row
end

def bool_if_tests(row)
  starting_tests('bool if')
  row = true_if_test(row)
  row = false_if_test(row)

  starting_tests('bool if else')
  row = true_if_else_test(row)
  row = false_if_else_test(row)

  starting_tests('bool if and multiple conditions')
  row = true_and_true_if_multiple_condition_test(row)
  row = true_and_false_if_multiple_condition_test(row)
  row = false_and_true_if_multiple_condition_test(row)
  row = false_and_false_if_multiple_condition_test(row)

  starting_tests('bool if or multiple conditions')
  row = true_or_true_if_multiple_condition_test(row)
  row = true_or_false_if_multiple_condition_test(row)
  row = false_or_true_if_multiple_condition_test(row)
  row = false_or_false_if_multiple_condition_test(row)

  starting_tests('bool if else and multiple conditions')
  row = true_and_true_if_else_multiple_condition_test(row)
  row = true_and_false_if_else_multiple_condition_test(row)
  row = false_and_true_if_else_multiple_condition_test(row)
  row = false_and_false_if_else_multiple_condition_test(row)

  starting_tests('bool if or multiple conditions')
  row = true_or_true_if_else_multiple_condition_test(row)
  row = true_or_false_if_else_multiple_condition_test(row)
  row = false_or_true_if_else_multiple_condition_test(row)
  false_or_false_if_else_multiple_condition_test(row)
end

# Equals if tests
def equals_if_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 1 -eq 1 ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('equals if test')
  row
end

def not_equals_if_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 2 -eq 1 ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('not equals if test')
  row
end

def equals_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 1 -eq 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('equals if else test')
  row
end

def not_equals_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 2 -eq 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not equals if else test')
  row
end

def equals_if_tests(row)
  starting_tests('equals if')
  row = equals_if_test(row)
  row = not_equals_if_test(row)
  row = equals_if_else_test(row)
  not_equals_if_else_test(row)
end

# Greater than if tests
def gt_if_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 2 -gt 1 ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('gt if test')
  row
end

def not_gt_if_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 1 -gt 2 ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('not gt if test')
  row
end

def gt_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 2 -gt 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('gt if else test')
  row
end

def not_gt_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 1 -gt 2 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not gt if else test')
  row
end

def gt_if_tests(row)
  starting_tests('gt if')
  row = gt_if_test(row)
  row = not_gt_if_test(row)
  row = gt_if_else_test(row)
  not_gt_if_else_test(row)
end

# Less than if tests
def lt_if_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 1 -lt 2 ]; then echo hello; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('lt if test')
  row
end

def not_lt_if_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 2 -lt 1 ]; then echo hello; fi')
  row += 1
  assert_check_new_row(row)
  test_passed('not lt if test')
  row
end

def lt_if_else_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ 1 -lt 2 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('lt if else test')
  row
end

def not_lt_if_else_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ 2 -lt 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('not lt if else test')
  row
end

def lt_if_tests(row)
  starting_tests('lt if')
  row = lt_if_test(row)
  row = not_lt_if_test(row)
  row = lt_if_else_test(row)
  not_lt_if_else_test(row)
end

def if_variable_test(row)
  assert_check_new_row(row)
  @tty.send_line('VAL=1')
  row += 1
  @tty.send_line_exact('if [ $VAL -eq 1 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('if variable test')
  row
end

def if_variables_test(row)
  assert_check_new_row(row)
  @tty.send_line('VAL2=2')
  row += 1
  @tty.send_line_exact('if [ $VAL -eq $VAL2 ]; then echo hello; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  test_passed('if variables test')
  row
end

def if_variable_tests(row)
  starting_tests('if variable')
  row = if_variable_test(row)
  if_variables_test(row)
end

def true_if_elif_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ false ]; then echo hello; elif [ true ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hey')
  row += 1
  test_passed('true if elif test')
  row
end

def false_if_elif_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false ]; then echo hello; elif [ false ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false if elif test')
  row
end

def true_and_true_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ false ]; then echo hello; elif [ true && true ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hey')
  row += 1
  test_passed('true and elif multiple condition test')
  row
end

def true_and_false_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ true && false ]; then echo hello; elif [ true && false ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  test_passed('true and elif multiple condition test')
  row
end

def false_and_true_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && true ]; then echo hello; elif [ false && true ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and elif multiple condition test')
  row
end

def false_and_false_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false && false ]; then echo hello; elif [ false && false ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false and elif multiple condition test')
  row
end

def true_or_true_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ false ]; then echo hello; elif [ false || true ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hey')
  row += 1
  test_passed('true or elif multiple condition test')
  row
end

def true_or_false_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)

  @tty.send_line_exact('if [ false ]; then echo hello; elif [ true || false ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hey')
  row += 1
  test_passed('true or elif multiple condition test')
  row
end

def false_or_true_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false ]; then echo hello; elif [ false || true]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hey')
  row += 1
  assert_check_new_row(row)
  test_passed('false or elif multiple condition test')
  row
end

def false_or_false_if_elif_multiple_condition_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('if [ false ]; then echo hello; elif [ false || false ]; then echo hey; else echo hi; fi')
  row += 1
  @tty.assert_row(row, 'hi')
  row += 1
  assert_check_new_row(row)
  test_passed('false or elif multiple condition test')
  row
end

def elif_tests(row)
  starting_tests('elif tests')
  row = true_if_elif_test(row)
  row = false_if_elif_test(row)

  row = true_and_true_if_elif_multiple_condition_test(row)
  row = true_and_false_if_elif_multiple_condition_test(row)
  row = false_and_true_if_elif_multiple_condition_test(row)
  row = false_and_false_if_elif_multiple_condition_test(row)

  row = true_or_true_if_elif_multiple_condition_test(row)
  row = true_or_false_if_elif_multiple_condition_test(row)
  row = false_or_true_if_elif_multiple_condition_test(row)
  false_or_false_if_elif_multiple_condition_test(row)
end

def if_tests(row)
  row = bool_if_tests(row)
  row = equals_if_tests(row)
  row = gt_if_tests(row)
  row = lt_if_tests(row)
  row = if_variable_tests(row)
  elif_tests(row)
end
