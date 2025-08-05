# frozen_string_literal: true

require 'ttytest'

# Test related constants
WC_C_LENGTH_I = 273
WC_C_LENGTH_NO_LOCAL_FILES = '239'
WC_C_LENGTH = WC_C_LENGTH_I.to_s

SLEEP_TIME = 0.2

LS_LINES = 3
LS_LINES_NO_Z = LS_LINES

LS_ITEMS = 23
LS_ITEMS_NO_Z = LS_ITEMS - 3
LS_ITEMS_NO_LOCAL_FILES = LS_ITEMS - 3

LS_FIRST_ITEM = 'AUTOCOMPLETIONS_CORPUS'

HEAD_ONE_ITEM = 'tests'

TAB_AUTOCOMPLETE_ROWS = 11

# Shell compiler option related constants
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
  @is_custom_prompt = is_custom_prompt
  @show_directory = prompt_directory_option == PROMPT_DIRECTORY_NONE
  @start_column = PROMPT_LENGTH

  case prompt_directory_option
  when PROMPT_DIRECTORY_NORMAL
    @start_column += Dir.pwd.length
  when PROMPT_DIRECTORY_SHORT
    @start_column += Dir.pwd.length - HOME_LENGTH
  when PROMPT_DIRECTORY_NONE
    @start_column -= 1 if prompt_user_option != PROMPT_SHOW_USER_NONE
  end

  @user = ENV['USER']
  @show_user = prompt_user_option == PROMPT_SHOW_USER_NORMAL
  @start_column += @user.length + 1 if @show_user

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
