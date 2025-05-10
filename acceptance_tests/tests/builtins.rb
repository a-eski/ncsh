# frozen_string_literal: true

require './acceptance_tests/tests/common'

def help_test(row)
  assert_check_new_row(row)
  @tty.send_line('help')
  row += 1
  @tty.assert_contents_at row, row + 13, <<~TERM
    ncsh Copyright (C) 2025 Alex Eski
    This program comes with ABSOLUTELY NO WARRANTY.
    This is free software, and you are welcome to redistribute it under certain conditions.

    ncsh help:
    Builtin Commands: {command} {args}
    q:                    To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.
    cd/z:                 You can change directory with cd or z.
    echo:                 You can write things to the screen using echo.
    history:              You can see your command history using the history command.
    history count:        You can see the number of entries in your history with history count command.
    pwd:                  Prints the current working directory.
    alex /shells/ncsh ❱
  TERM
  row += 12
  test_passed('Help test')
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

def echo_tests(row)
  row = basic_echo_test(row)
  row = quote_echo_test(row)
  row = no_newline_echo_test(row)
  multiple_echo_tests(row)
end

def builtins_tests(row)
  starting_tests('builtins')

  # row = help_test(row)
  echo_tests(row)
end
