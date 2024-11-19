#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'

def assert_check_new_row(row)
  @tty.assert_row_starts_with(row, "#{ENV['USER']}:")
  @tty.assert_row_like(row, 'ncsh')
  @tty.assert_row_ends_with(row, '$')
  @tty.assert_cursor_position(18, row)
end

@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)

row = 0

# # # # Basic Tests # # # #
puts 'Starting basic tests'

@tty.assert_row_starts_with(row, 'ncsh: startup time: ')
row += 1

assert_check_new_row(row)
@tty.send_keys_one_at_a_time(%(ls))
@tty.assert_cursor_position(20, 1)
@tty.send_newline
@tty.assert_row_ends_with(row, 'ls')
row += 1
@tty.assert_row_starts_with(row, 'LICENSE')
row = 9

assert_check_new_row(row)
@tty.send_keys_one_at_a_time(%(echo hello))
@tty.send_newline
row += 1
@tty.assert_row(row, 'hello')
row += 1

assert_check_new_row(row)
@tty.send_keys_one_at_a_time(%(lss)) # send a bad command
@tty.send_newline
row += 1
@tty.assert_row(row, 'ncsh: Could not find command or directory: No such file or directory')
row += 1

puts 'Starting backspace tests'

# end of line backspace
assert_check_new_row(row)
@tty.send_keys_one_at_a_time(%(l))
@tty.send_keys(TTYtest::BACKSPACE)
assert_check_new_row(row)

# multiple end of line backspaces
@tty.send_keys_one_at_a_time(%(lsssss))
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.assert_row_ends_with(row, '$ ls')
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
@tty.send_newline
row += 1
@tty.assert_row(row, 'hello')
row += 1

# midline backspace
assert_check_new_row(row)
@tty.send_keys_one_at_a_time(%(lsssss))
@tty.assert_cursor_position(24, row)
@tty.send_keys(TTYtest::LEFT_ARROW)
@tty.send_keys(TTYtest::LEFT_ARROW)
@tty.assert_cursor_position(22, row)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.assert_cursor_position(18, row)
@tty.assert_row_ends_with(row, '$ ss')
@tty.send_keys(TTYtest::RIGHT_ARROW)
@tty.send_keys(TTYtest::RIGHT_ARROW)
@tty.assert_cursor_position(20, row)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.assert_cursor_position(18, row)
@tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
@tty.send_newline
row += 1
@tty.assert_row(row, 'hello')
row += 1

# puts 'Starting delete tests'

assert_check_new_row(row)
@tty.send_keys_one_at_a_time('s')
@tty.assert_cursor_position(19, row)
@tty.send_keys(TTYtest::LEFT_ARROW)
@tty.assert_cursor_position(18, row)

# assert_check_new_row(17)
# @tty.send_keys_one_at_a_time(%(lssss))
# @tty.assert_cursor_position(68, 17)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.assert_cursor_position(63, 17)
# @tty.send_keys(TTYtest::DELETE)
# @tty.assert_cursor_position(63, 17)
# @tty.send_keys(TTYtest::DELETE)
# @tty.send_keys(TTYtest::DELETE)
# @tty.send_keys(TTYtest::DELETE)
# @tty.send_keys(TTYtest::DELETE)
# assert_check_new_row(17)

# puts 'Starting multiline tests'
# puts 'Starting history tests'
# puts 'Starting autocomplete tests'
# puts 'Starting pipe tests'
# puts 'Starting output redirection tests'
# puts 'Starting arrow tests'

# assert_check_new_row(17)
# @tty.send_keys(TTYtest::RIGHT_ARROW)
# assert_check_new_row(17)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# assert_check_new_row(17)

puts 'All tests passed!'
