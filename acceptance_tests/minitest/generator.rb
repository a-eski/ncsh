# frozen_string_literal: true

require 'ttytest'

ROW_START = 2 # to skip version and other startup stuff

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
    'false || true || false'
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
    true
  ]

  conditions.each_with_index do |condition, i|
    result = results[i]

    name = condition.gsub(' ', '_').gsub('&&', 'and').gsub('||', 'or')

    define_method("test_if_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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

    define_method("test_if_elif_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')
      tty.send_line_exact("if [ false ]; then echo hello; elif [ #{condition} ]; then echo hey; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row_ends_with(row, ' ❱ ')
      end
    end

    define_method("test_if_elif_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
      row = ROW_START
      tty.assert_row_ends_with(row, ' ❱ ')
      tty.send_line_exact("if [ false ]; then echo hello; elif [ #{condition} ]; then echo hey; else echo hi; fi")
      row += 1
      if result
        tty.assert_row(row, 'hey')
      else
        tty.assert_row(row, 'hi')
      end
    end
  end
end

def generate_if_math_tests
  conditions = [
    '1 -eq 1',
    '2 -eq 1',
    '1 -eq 2',
    '1 -lt 1',
    '2 -lt 1',
    '1 -lt 2',
    '1 -gt 1',
    '1 -gt 2',
    '2 -gt 1'
  ]

  results = [
    true,
    false,
    false,
    false,
    false,
    true,
    false,
    false,
    true
  ]

  conditions.each_with_index do |condition, i|
    result = results[i]

    name = condition.gsub(' ', '_')
                    .gsub('-', '').gsub('1', 'one').gsub('2', 'two')

    define_method("test_if_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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

    define_method("test_if_elif_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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

    define_method("test_if_elif_else_#{name}_#{i}") do
      tty = TTYtest.new_terminal(%(../bin/ncsh), width: 180, height: 160)
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
  end
end
