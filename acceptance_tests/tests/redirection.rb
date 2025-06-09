# frozen_string_literal: true

require './acceptance_tests/tests/common'

def basic_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t.txt', SLEEP_TIME)
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t.txt')
  row += 1
  test_passed('Basic output redirection test')
  row
end

def piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort -r > t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort -r > t2.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t2.txt))
  row += 1
  @tty.assert_row_starts_with(row, HEAD_ONE_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t2.txt')
  row += 1
  test_passed('Piped output redirection test')
  row
end

def multiple_piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort | wc -c > t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls | sort | wc -c > t3.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(head -1 t3.txt))
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t3.txt')
  row += 1
  test_passed('Multiple piped output redirection test')
  row
end

def stdout_redirection_append_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls | sort | wc -c > t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('ls | sort | wc -c >> t3.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t3.txt')
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.assert_row(row, (WC_C_LENGTH.to_i + 't3.txt'.length + 1).to_s)
  row += 1
  @tty.send_line('rm t3.txt')
  row += 1

  test_passed('Stdout redirection append test')
  row
end

def stdout_redirection_tests(row)
  starting_tests('stdout redirection')

  row = basic_stdout_redirection_test(row)
  row = piped_stdout_redirection_test(row)
  row = multiple_piped_stdout_redirection_test(row)
  stdout_redirection_append_test(row)
end

def basic_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t.txt', SLEEP_TIME)
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('sort < t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_ITEMS
  assert_check_new_row(row)
  @tty.send_line('rm t.txt')
  row += 1
  test_passed('Basic input redirection test')
  row
end

def piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls > t2.txt))
  row += 1
  assert_check_new_row(row)
  @tty.send_line_then_sleep('sort | wc -c < t2.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(sort | wc -c < t2.txt))
  row += 1
  @tty.assert_row_starts_with(row, (WC_C_LENGTH.to_i + 't2.txt'.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t2.txt')
  row += 1
  test_passed('Piped input redirection test')
  row
end

def multiple_piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls > t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(ls > t3.txt))
  row += 1
  @tty.send_line_then_sleep('sort | head -1 | wc -l < t3.txt', SLEEP_TIME)
  @tty.assert_row_ends_with(row, %(sort | head -1 | wc -l < t3.txt))
  row += 1
  @tty.assert_row(row, '1')
  row += 1
  assert_check_new_row(row)
  @tty.send_line('rm t3.txt')
  row += 1
  test_passed('Multiple piped input redirection test')
  row
end

def stdin_redirection_tests(row)
  starting_tests('stdin redirection')

  row = basic_stdin_redirection_test(row)
  row = piped_stdin_redirection_test(row)
  multiple_piped_stdin_redirection_test(row)
end

def redirection_tests(row)
  row = stdout_redirection_tests(row)
  stdin_redirection_tests(row)
end
