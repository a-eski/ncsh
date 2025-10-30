# frozen_string_literal: true

require './acceptance_tests/tests/common'

def for_expansion_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('for file in src/*.c; do echo $file done')
  row += 1
  @tty.assert_row(row, 'src/alias.c')
  row += 1
  @tty.assert_row(row, 'src/arena.c')
  row += 1
  @tty.assert_row(row, 'src/conf.c')
  row += 1
  @tty.assert_row(row, 'src/env.c')
  row += 1
  @tty.assert_row(row, 'src/main.c')
  row += 1
  @tty.assert_row(row, 'src/unity.c')
  row += 1
  @tty.assert_row(row, 'src/vars.c')
  row += 1
  test_passed('for expansion')
  row
end

def for_expansion_ending_semic_test(row)
  assert_check_new_row(row)
  @tty.send_line_exact('for file in src/*.c; do echo $file; done')
  row += 1
  @tty.assert_row(row, 'src/alias.c')
  row += 1
  @tty.assert_row(row, 'src/arena.c')
  row += 1
  @tty.assert_row(row, 'src/conf.c')
  row += 1
  @tty.assert_row(row, 'src/env.c')
  row += 1
  @tty.assert_row(row, 'src/main.c')
  row += 1
  @tty.assert_row(row, 'src/unity.c')
  row += 1
  @tty.assert_row(row, 'src/vars.c')
  row += 1
  test_passed('for expansion ending semic')
  row
end

def for_tests(row)
  starting_tests('for')
  row = for_expansion_test(row)
  for_expansion_ending_semic_test(row)
end
