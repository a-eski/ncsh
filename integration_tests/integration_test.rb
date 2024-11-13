#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'

BACKSPACE = 127.chr
TAB = 9.chr
CTRLF = 6.chr
CTRLC = 3.chr

def assert_check_new_row(row)
  @tty.assert_row_starts_with(row, "#{ENV['USER']}:")
  @tty.assert_row_like(row, 'ncsh')
  @tty.assert_row_ends_with(row, '$')
end

def send_keys_newline
  @tty.send_keys(%(\n))
end

def send_keys_ls
  @tty.send_keys('l')
  @tty.send_keys('s')
end

def send_keys_echo
  @tty.send_keys('e')
  @tty.send_keys('c')
  @tty.send_keys('h')
  @tty.send_keys('o')
  @tty.send_keys(' ')
  @tty.send_keys('h')
  @tty.send_keys('e')
  @tty.send_keys('l')
  @tty.send_keys('l')
  @tty.send_keys('o')
end

def send_keys_bad_command
  @tty.send_keys('l')
  @tty.send_keys('s')
  @tty.send_keys('s')
end

@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)

puts 'Starting basic tests'

@tty.assert_row_starts_with(0, 'ncsh: startup time: ')

assert_check_new_row(1)
send_keys_ls
send_keys_newline
@tty.assert_row_ends_with(1, 'ls')
@tty.assert_row_starts_with(2, 'LICENSE')
@tty.assert_row_starts_with(8, '_docker.txt')

assert_check_new_row(9)
send_keys_echo
send_keys_newline
@tty.assert_row(10, 'hello')

assert_check_new_row(11)
send_keys_bad_command
send_keys_newline
@tty.assert_row(12, 'ncsh: Could not find command or directory: No such file or directory')

assert_check_new_row(13)

# puts "Starting backspace tests"

# puts "Starting delete tests"

puts 'All tests passed!'
