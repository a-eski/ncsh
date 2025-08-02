# frozen_string_literal: true

require './acceptance_tests/tests/common'

def basic_stdout_redirection_test(row)
  assert_check_new_row(row)
  file = 't.txt'
  @tty.send_line_then_sleep("ls > #{file}", SLEEP_TIME)
  row += 1
  @tty.assert_file_exists(file)
  @tty.assert_file_contains(file, LS_FIRST_ITEM)
  @tty.assert_file_has_line_count(file, LS_ITEMS)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('head -1 t.txt', SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
  row += 1
  test_passed('Basic output redirection test')
  row
end

def piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  file = 't2.txt'
  @tty.send_line_then_sleep("ls | sort -r > #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "ls | sort -r > #{file}")
  row += 1
  @tty.assert_file_exists(file)
  @tty.assert_file_contains(file, HEAD_ONE_ITEM)
  @tty.assert_file_has_line_count(file, LS_ITEMS)
  assert_check_new_row(row)
  @tty.send_line_then_sleep("head -1 #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "head -1 #{file}")
  row += 1
  @tty.assert_row_starts_with(row, HEAD_ONE_ITEM)
  row += 1
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
  row += 1
  test_passed('Piped output redirection test')
  row
end

def multiple_piped_stdout_redirection_test(row)
  assert_check_new_row(row)
  file = 't3.txt'
  @tty.send_line_then_sleep("ls | sort | wc -c > #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "ls | sort | wc -c > #{file}")
  row += 1
  @tty.assert_file_exists(file)
  len = (WC_C_LENGTH.to_i + file.length + 1).to_s
  @tty.assert_file_contains(file, len)
  @tty.assert_file_has_line_count(file, 1)
  assert_check_new_row(row)
  @tty.send_line_then_sleep("head -1 #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "head -1 #{file}")
  row += 1
  @tty.assert_row(row, len)
  row += 1
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
  row += 1
  test_passed('Multiple piped output redirection test')
  row
end

def stdout_redirection_append_test(row)
  assert_check_new_row(row)
  file = 't3.txt'
  @tty.send_line_then_sleep("ls | sort | wc -c > #{file}", SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep("ls | sort | wc -c >> #{file}", SLEEP_TIME)
  row += 1
  @tty.assert_file_exists(file)
  len = (WC_C_LENGTH.to_i + file.length + 1).to_s
  @tty.assert_file_contains(file, len)
  @tty.assert_file_has_line_count(file, 2)
  @tty.send_line("cat #{file}")
  row += 1
  @tty.assert_row(row, len)
  row += 1
  @tty.assert_row(row, len)
  row += 1
  @tty.send_line("rm #{file}")
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
  file = 't.txt'
  @tty.send_line_then_sleep("ls > #{file}", SLEEP_TIME)
  row += 1
  @tty.assert_file_exists(file)
  @tty.assert_file_contains(file, LS_FIRST_ITEM)
  @tty.assert_file_has_line_count(file, LS_ITEMS)
  assert_check_new_row(row)
  @tty.send_line_then_sleep("sort < #{file}", SLEEP_TIME)
  row += 1
  @tty.assert_row_starts_with(row, LS_FIRST_ITEM)
  row += LS_ITEMS
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
  row += 1
  test_passed('Basic input redirection test')
  row
end

def piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  file = 't2.txt'
  @tty.send_line_then_sleep("ls > #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "ls > #{file}")
  row += 1
  @tty.assert_file_exists(file)
  @tty.assert_file_contains(file, LS_FIRST_ITEM)
  @tty.assert_file_has_line_count(file, LS_ITEMS)
  assert_check_new_row(row)
  @tty.send_line_then_sleep("sort | wc -c < #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "sort | wc -c < #{file}")
  row += 1
  @tty.assert_row_starts_with(row, (WC_C_LENGTH.to_i + file.length + 1).to_s)
  row += 1
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
  row += 1
  test_passed('Piped input redirection test')
  row
end

def multiple_piped_stdin_redirection_test(row)
  assert_check_new_row(row)
  file = 't3.txt'
  @tty.send_line_then_sleep("ls > #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "ls > #{file}")
  row += 1
  @tty.assert_file_exists(file)
  @tty.assert_file_contains(file, LS_FIRST_ITEM)
  @tty.assert_file_has_line_count(file, LS_ITEMS)
  @tty.send_line_then_sleep("sort | head -1 | wc -l < #{file}", SLEEP_TIME)
  @tty.assert_row_ends_with(row, "sort | head -1 | wc -l < #{file}")
  row += 1
  @tty.assert_row(row, '1')
  row += 1
  assert_check_new_row(row)
  @tty.send_line("rm #{file}")
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
