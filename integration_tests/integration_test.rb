#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'

def assert_check_new_row(row)
  if (ENV['USER'] == nil)
    @tty.assert_row_starts_with(row, "(null)")
  else
    @tty.assert_row_starts_with(row, "#{ENV['USER']}:")
  @tty.assert_row_like(row, 'ncsh')
  @tty.assert_row_ends_with(row, '$')
  @tty.assert_cursor_position(63, row)
end

@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)

# # # # Basic Tests # # # #
puts 'Starting basic tests'

@tty.assert_row_starts_with(0, 'ncsh: startup time: ')

assert_check_new_row(1)
@tty.send_keys_one_at_a_time(%(ls))
@tty.assert_cursor_position(65, 1)
@tty.send_newline
@tty.assert_row_ends_with(1, 'ls')
@tty.assert_row_starts_with(2, 'LICENSE')
@tty.assert_row_starts_with(8, '_docker.txt')

assert_check_new_row(9)
@tty.send_keys_one_at_a_time(%(echo hello))
@tty.send_newline
@tty.assert_row(10, 'hello')

assert_check_new_row(11)
@tty.send_keys_one_at_a_time(%(lss)) # send a bad command
@tty.send_newline
@tty.assert_row(12, 'ncsh: Could not find command or directory: No such file or directory')

puts 'Starting backspace tests'

# end of line backspace
assert_check_new_row(13)
@tty.send_keys_one_at_a_time(%(l))
@tty.send_keys(TTYtest::BACKSPACE)
assert_check_new_row(13)

# multiple end of line backspaces
@tty.send_keys_one_at_a_time(%(lsssss))
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.assert_row_ends_with(13, '$ ls')
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
@tty.send_newline
@tty.assert_row(14, 'hello')

# midline backspace
assert_check_new_row(15)
@tty.send_keys_one_at_a_time(%(lsssss))
@tty.assert_cursor_position(69, 15)
@tty.send_keys(TTYtest::LEFT_ARROW)
@tty.send_keys(TTYtest::LEFT_ARROW)
@tty.assert_cursor_position(67, 15)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.assert_cursor_position(63, 15)
@tty.assert_row_ends_with(15, '$ ss')
@tty.send_keys(TTYtest::RIGHT_ARROW)
@tty.send_keys(TTYtest::RIGHT_ARROW)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys(TTYtest::BACKSPACE)
@tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
@tty.send_newline
@tty.assert_row(16, 'hello')

# puts 'Starting delete tests'

# assert_check_new_row(17)
# @tty.send_keys_one_at_a_time('s')
# @tty.assert_cursor_position(64, 17)
# @tty.send_keys(TTYtest::LEFT_ARROW)
# @tty.assert_cursor_position(63, 17)
# @tty.send_keys_one_at_a_time('^[[5~')
# @tty.assert_cursor_position(63, 17)

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
