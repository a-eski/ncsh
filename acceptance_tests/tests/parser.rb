# frozen_string_literal: true

require './acceptance_tests/tests/common'

def comment_test(row)
  assert_check_new_row(row)
  @tty.send_line('echo hello # this is a comment')
  row += 1
  @tty.assert_row(row, 'hello')
  row += 1
  test_passed('Comment test')
  row
end

def parser_tests(row)
  starting_tests('parser')
  comment_test(row)
end
