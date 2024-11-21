# #!/usr/bin/env ruby
# frozen_string_literal: true

require 'minitest/autorun'
require 'ttytest'

module Ncsh
  # helper class for testing ncsh
  class NcshTestHelpers
    START_COL = 20

    def assert_check_new_row(row)
      @tty.assert_row_starts_with(row, "#{ENV['USER']}->")
      @tty.assert_row_like(row, 'ncsh')
      @tty.assert_row_ends_with(row, '>')
      @tty.assert_cursor_position(START_COL, row)
    end
  end

  # tests class for testing ncsh
  class NcshTests < Minitest::Test
    def basic_tests
      @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)
      row = 0

      @tty.assert_row_starts_with(row, 'ncsh: startup time: ')
      row += 1

      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time(%(ls))
      @tty.assert_cursor_position(START_COL + 2, 1)
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
    end

    def backspace_tests
      @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)
      row = 0

      # end of line backspace
      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time(%(l))
      @tty.send_backspace
      assert_check_new_row(row)

      # multiple end of line backspaces
      @tty.send_keys_one_at_a_time(%(lsssss))
      @tty.send_backspaces(4)
      @tty.assert_row_ends_with(row, '> ls')
      @tty.send_backspaces(2)
      @tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
      @tty.send_newline
      row += 1
      @tty.assert_row(row, 'hello')
      row += 1

      # midline backspace
      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time(%(lsssss))
      @tty.assert_cursor_position(START_COL + 6, row)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.assert_cursor_position(START_COL + 4, row)
      @tty.send_backspaces(4)
      @tty.assert_cursor_position(START_COL, row)
      @tty.assert_row_ends_with(row, '> ss')
      @tty.send_keys(TTYtest::RIGHT_ARROW)
      @tty.send_keys(TTYtest::RIGHT_ARROW)
      @tty.assert_cursor_position(START_COL + 2, row)
      @tty.send_backspaces(2)
      @tty.assert_cursor_position(START_COL, row)
      @tty.send_keys_one_at_a_time(%(echo hello)) # make sure buffer is properly formed after backspaces
      @tty.send_newline
      row += 1
      @tty.assert_row(row, 'hello')
    end

    def delete_tests
      @tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 80, height: 24)
      row = 0

      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time('s')
      @tty.assert_cursor_position(START_COL + 1, row)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.assert_cursor_position(START_COL, row)
      @tty.send_delete
      assert_check_new_row(row)

      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time(%(lssss))
      @tty.assert_cursor_position(START_COL + 5, row)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.assert_cursor_position(START_COL + 1, row)
      @tty.send_delete
      @tty.assert_cursor_position(START_COL + 1, row)
      @tty.send_deletes(3)
      @tty.send_keys(TTYtest::LEFT_ARROW)
      @tty.send_delete
      assert_check_new_row(row)
      @tty.send_keys_one_at_a_time(%(echo hello))
      @tty.send_newline
      row += 1
      @tty.assert_row(row, 'hello')
      row += 1
      assert_check_new_row(row)
    end

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
  end
end
