# frozen_string_literal: true

require 'minitest/autorun'

module NcshTests
  # tests for if ... elif ... else ... fi
  class IfTests < Minitest::Test
    def self.startup
      File.delete('./ncsh_history_test')
      File.delete('./_z_database.bin')
    end

    generate_if_bool_tests
    generate_if_math_tests
  end
end
