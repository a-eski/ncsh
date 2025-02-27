# frozen_string_literal: true

PROMPT_DIRECTORY_NORMAL = 0
PROMPT_DIRECTORY_SHORT = 1
PROMPT_DIRECTORY_NONE = 2

PROMPT_SHOW_USER_NORMAL = 0
PROMPT_SHOW_USER_NONE = 1

PROMPT_LENGTH = 3
HOME_LENGTH = 5

# Setup

# setup_tests sets instance variables needed throughout the tests.
# setup_tests covers the different permutations of prompts for ncsh.
#
# Permutations
#   {user} {directory} {prompt}
#   {directory} {prompt}
#
#   {user} {short_directory} {prompt}
#   {short_directory} {prompt}
#
#   {user} {prompt}
#
#   {prompt}
#
def setup_tests(prompt_directory_option, prompt_user_option, is_custom_prompt)
  directory = Dir.pwd
  @user = ENV['USER']
  @show_directory = prompt_directory_option == PROMPT_DIRECTORY_NONE
  @show_user = prompt_user_option == PROMPT_SHOW_USER_NORMAL
  @start_column = PROMPT_LENGTH
  @is_custom_prompt = is_custom_prompt

  case prompt_directory_option
  when PROMPT_DIRECTORY_NORMAL
    @start_column += directory.length
  when PROMPT_DIRECTORY_SHORT
    @start_column += directory.length - HOME_LENGTH
  when PROMPT_DIRECTORY_NONE
    @start_column -= 1 if prompt_user_option != PROMPT_SHOW_USER_NONE
  end

  case prompt_user_option
  when PROMPT_SHOW_USER_NORMAL
    @start_column += @user.length + 1
  end

  return unless @is_custom_prompt

  @start_column -= 2
end

def starting_tests(test)
  puts "===== Starting #{test} tests ====="
end

def test_passed(test)
  puts "#{test} passed"
end

# Assertion
def assert_check_new_row(row)
  @tty.assert_row_starts_with(row, "#{@user} ") if @show_user
  @tty.assert_row_like(row, 'ncsh') unless @show_directory
  if @is_custom_prompt
    @tty.assert_row_ends_with(row, '$')
  else
    @tty.assert_row_ends_with(row, ' ❱ ')
  end
  @tty.assert_cursor_position(@start_column, row)
end

# Tests
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
  @tty.assert_row_starts_with(row, 'ncsh: startup time: ')
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
  starting_tests 'startup tests'

  row = z_database_new_test row if run_z_database_new_tests
  row = startup_test row
  row = newline_sanity_test row

  empty_line_sanity_test row
end
