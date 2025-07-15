# frozen_string_literal: true

require './acceptance_tests/tests/common'

def help_test(row)
  assert_check_new_row(row)
  @tty.send_line('help > t.txt')
  row += 1
  @tty.send_line('cat t.txt | head -1')
  row += 1
  @tty.assert_row_starts_with(row, 'ncsh')
  row += 1
  @tty.send_line('rm t.txt')
  row += 1
  test_passed('help test')
  row
end

def help_pipe_test(row)
  assert_check_new_row(row)
  @tty.send_line('help | head -1')
  row += 1
  @tty.assert_row_like(row, 'ncsh')
  row += 1
  test_passed('help pipe test')
  row
end

def help_tests(row)
  starting_tests('help')
  # row = help_test(row)
  # help_pipe_test(row)
  row
end

def basic_echo_test(row)
  assert_check_new_row(row)
  @tty.send_keys_one_at_a_time(%(echo hello))
  @tty.assert_cursor_position(@start_column + 10, row)
  @tty.send_newline
  @tty.assert_row_ends_with(row, 'echo hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('basic echo test')
  row
end

def quote_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line("echo 'hello'")
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  @tty.send_line('echo "hello"')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  @tty.send_line('echo `hello`')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1

  test_passed('quote echo test')
  row
end

def assert_check_new_row_no_newline_echo(start, row)
  @tty.assert_row_starts_with(row, start)
  @tty.assert_row_like(row, "#{@user} ") if @show_user
  @tty.assert_row_like(row, 'ncsh') unless @show_directory
  if @is_custom_prompt
    @tty.assert_row_ends_with(row, '$')
  else
    @tty.assert_row_ends_with(row, ' ❱ ')
  end
end

def no_newline_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo -n hello')
  row += 1
  assert_check_new_row_no_newline_echo('hello', row)
  row += 1
  @tty.send_newline
  test_passed('no newline echo test')
  row
end

def multiple_echo_tests(row)
  assert_check_new_row(row)
  @tty.send_line('echo hello && echo hello && echo echo hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  @tty.assert_row(row, 'echo hello')
  row += 1
  test_passed('multiple echo test')
  row
end

def empty_echo_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo')
  row += 1
  @tty.assert_row_is_empty(row)
  row += 1
  test_passed('empty echo test')
  row
end

def empty_echo_no_newline_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo -n')
  row += 1
  assert_check_new_row(row)
  test_passed('empty echo no newline test')
  row
end

def echo_tests(row)
  starting_tests('echo')
  row = basic_echo_test(row)
  row = quote_echo_test(row)
  row = no_newline_echo_test(row)
  row = multiple_echo_tests(row)
  row = empty_echo_test(row)
  empty_echo_no_newline_test(row)
end

def nothing_to_kill_test(row)
  assert_check_new_row(row)
  @tty.send_line('kill')
  row += 1
  @tty.assert_row_like(row, 'nothing to kill')
  row += 1
  test_passed('nothing to kill test')
  row
end

def kill_test(row)
  assert_check_new_row(row)
  @tty.send_line('kill 10000')
  row += 1
  @tty.assert_row_like(row, 'could not kill')
  row += 1
  test_passed('kill test')
  row
end

def kill_tests(row)
  starting_tests('kill')
  row = nothing_to_kill_test(row)
  kill_test(row)
end

def pwd_test(row)
  @tty.send_line('pwd')
  row += 1
  @tty.assert_row(row, Dir.pwd)
  row += 1
  test_passed('pwd test')
  row
end

def builtins_tests(row)
  row = help_tests(row)
  row = echo_tests(row)
  row = kill_tests(row)
  pwd_test(row)
end
