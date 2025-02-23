# frozen_string_literal: true

def assert_check_syntax_error(row, input)
  assert_check_new_row(row)
  @tty.send_line(input)
  row += 1
  @tty.assert_row_starts_with(row, 'ncsh: Invalid syntax:')
  row += 1
  row
end

def operators_invalid_syntax_first_position_test(row)
  # tries sending operator as only character to ensure invalid syntax is shown to user
  row = assert_check_syntax_error(row, '|') # pipe
  row = assert_check_syntax_error(row, '>') # output redirection
  row = assert_check_syntax_error(row, '>>') # output redirection append
  row = assert_check_syntax_error(row, '<') # input redirection
  row = assert_check_syntax_error(row, '2>') # error redirection
  row = assert_check_syntax_error(row, '2>>') # error redirection append
  row = assert_check_syntax_error(row, '&>') # output and error redirection
  row = assert_check_syntax_error(row, '&>>') # output and error redirection append
  row = assert_check_syntax_error(row, '&') # background job
  puts 'Invalid syntax in first position test passed'
  row
end

def operators_invalid_syntax_last_position_test(row)
  row = assert_check_syntax_error(row, 'ls |') # pipe
  row = assert_check_syntax_error(row, 'ls >') # output redirection
  row = assert_check_syntax_error(row, 'ls >>') # output redirection append
  row = assert_check_syntax_error(row, 'sort <') # input redirection
  row = assert_check_syntax_error(row, 'ls 2>') # error redirection
  row = assert_check_syntax_error(row, 'ls 2>>') # error redirection append
  row = assert_check_syntax_error(row, 'ls &>') # output and error redirection
  row = assert_check_syntax_error(row, 'ls &>>') # output and error redirection append
  puts 'Invalid syntax in last position test passed'
  row
end

def operators_invalid_syntax_not_last_position_test(row)
  row = assert_check_syntax_error(row, 'ls & ss') # background job not in last position
  puts 'Invalid syntax not in last position test passed'
  row
end

# invalid operator usage to ensure invalid syntax is shown to user
def syntax_error_tests(row)
  starting_tests 'syntax errors'

  row = operators_invalid_syntax_first_position_test row
  row = operators_invalid_syntax_last_position_test row
  operators_invalid_syntax_not_last_position_test row
end

def basic_stderr_redirection_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('lss 2> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t4.txt')
  row += 1
  @tty.assert_row_ends_with(row, 'No such file or directory')
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stderr redirection test passed'
  row
end

def stderr_redirection_append_test(row)
  assert_check_new_row(row)

  puts 'Stderr redirection append test passed'
  row
end

def stderr_redirection_tests(row)
  starting_tests 'sterr redirection'

  row = basic_stderr_redirection_test row
  stderr_redirection_append_test row
end

def basic_stdout_and_stderr_redirection_stderr_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('lss &> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line('cat t4.txt')
  row += 1
  @tty.assert_row_ends_with(row, 'No such file or directory')
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stdout and stderr redirection stderr test passed'
  row
end

def basic_stdout_and_stderr_redirection_stdout_test(row)
  assert_check_new_row(row)
  @tty.send_line_then_sleep('ls &> t4.txt', SLEEP_TIME)
  row += 1
  @tty.send_line_then_sleep('cat t4.txt | head -1', SLEEP_TIME)
  row += 1
  @tty.assert_row_ends_with(row, LS_FIRST_ITEM)
  row += 1
  @tty.send_line('rm t4.txt')
  row += 1

  puts 'Basic stdout and stderr redirection stdout test passed'
  row
end

def stdout_and_stderr_redirection_tests(row)
  starting_tests 'stdout and sterr redirection'

  row = basic_stdout_and_stderr_redirection_stderr_test row
  basic_stdout_and_stderr_redirection_stdout_test row
end

def syntax_tests(row)
  row = syntax_error_tests row
  row = stderr_redirection_tests row
  stdout_and_stderr_redirection_tests row
end
