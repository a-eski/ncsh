# frozen_string_literal: true

require 'minitest/autorun'

module NcshTests
  # tests for if [ #{condition} ]; then echo a; fi
  class IfTests < Minitest::Test
    generate_if_tests
  end
end
