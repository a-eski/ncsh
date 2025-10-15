# frozen_string_literal: true

require 'ttytest'

# generator: generate a variety of if, elif, else tests

ROW_START = 3 # to skip version and other startup stuff

def generate_if_bool_tests
  conditions = [
    'true',
    'false',
    'true && true',
    'true && false',
    'false && true',
    'false && false',
    'true || true',
    'true || false',
    'false || true',
    'false || false',
    'true && true && true',
    'true && true && false',
    'true && false && false',
    'false && false && false',
    'false && true && true',
    'false && false && true',
    'true && false && true',
    'false && true && false',
    'true || true || true',
    'true || true || false',
    'true || false || false',
    'false || false || false',
    'false || true || true',
    'false || false || true',
    'true || false || true',
    'false || true || false',
    'false || false || false || true',          # true
    'false || false || false || false',         # false
    'true && true && true && false',            # false
    'false || false || true || false',          # true
    'true && true && true && true',             # true
    'false || false || false || false || true', # true
    'true && true && true && true && true'      # true
  ]

  results = [
    true,
    false,
    true,
    false,
    false,
    false,
    true,
    true,
    true,
    false,
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
    false,
    true,
    true,
    true,
    true,
    true,
    false,
    false,
    true,
    true,
    true,
    true
  ]

  generate_if_tests(conditions, results)
end

def generate_if_math_tests
  conditions = [
    '1 -eq 1',
    '2 -eq 1',
    '1 -eq 2',
    '1 -lt 1',
    '2 -lt 1',
    '1 -lt 2',
    '1 -le 1',
    '2 -le 1',
    '1 -le 2',
    '1 -gt 1',
    '1 -gt 2',
    '2 -gt 1',
    '1 -ge 1',
    '1 -ge 2',
    '2 -ge 1'
  ]

  results = [
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    false,
    true,
    false,
    false,
    true,
    true,
    false,
    true
  ]

  generate_if_tests(conditions, results)
end

def generate_if_tests(conditions, results)
  conditions.each_with_index do |condition, i|
    result = results[i]

    name = condition.gsub(' ', '_')
                    .gsub('-', '')
                    .gsub('1', 'one')
                    .gsub('2', 'two')
                    .gsub('&&', 'and')
                    .gsub('||', 'or')

    define_method("test_if_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ #{condition} ]; then echo hello; fi")
      row += 1
      if result
        tty.assert_row(row, 'hello')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ #{condition} ]; then echo hello; else echo hi; fi")
      row += 1
      if result
        tty.assert_row(row, 'hello')
      else
        tty.assert_row(row, 'hi')
      end
    end

    define_method("test_if_first_elif_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ #{condition} ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_second_elif_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ #{condition} ]; then echo hello; elif [ 1 -eq 5 ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hello')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_first_elif_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ #{condition} ]; then echo hello; elif [ 1 -eq 5 ]; then echo hey; else echo hi; fi")
      row += 1
      if result
        tty.assert_row(row, 'hello')
      else
        tty.assert_row(row, 'hi')
      end
    end

    define_method("test_if_elif_else_multiple_conditions_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ false && true ]; then echo hello; elif [ #{condition} ]; then echo hey; else echo hi; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row(row, 'hi')
      end
    end

    define_method("test_if_second_elif_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ #{condition} ]; then echo hey; else echo hi; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row(row, 'hi')
      end
    end

    define_method("test_if_first_elif_multiple_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ #{condition} ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ 5 -lt 1 ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hello')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_second_elif_multiple_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ #{condition} ]; then echo hi; elif [ 5 -lt 1 ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hi')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_third_elif_multiple_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ #{condition} ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_fourth_elif_multiple_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ false && true ]; then echo hiya; elif [ #{condition} ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_first_elif_multiple_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ false ]; then echo a; elif [ #{condition} ]; then echo b; elif [ true && false ]; then echo c; else echo d; fi")
      row += 1
      if result
        tty.assert_row(row, 'b')
      else
        tty.assert_row(row, 'd')
      end
    end

    define_method("test_if_second_elif_multiple_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ #{condition} ]; then echo hey; else echo hallo; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row(row, 'hallo')
      end
    end

    define_method("test_if_third_elif_multiple_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ 1 -eq 5 ]; then echo a; elif [ 1 -eq 5 ]; then echo b; elif [ false || false ]; then echo c; elif [ #{condition} ]; then echo d; else echo e; fi")
      row += 1
      if result
        tty.assert_row(row, 'd')
      else
        tty.assert_row(row, 'e')
      end
    end

    define_method("test_if_multiple_second_elif_multiple_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 230, height: 160, use_return_for_newline: true)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')

      tty.send_line_exact("if [ false ]; then echo a; elif [ false ]; then echo b; elif [ #{condition} ]; then echo c; elif [ false && true ]; then echo d; else echo e; fi")
      row += 1
      if result
        tty.assert_row(row, 'c')
      else
        tty.assert_row(row, 'e')
      end
    end
  end
end
