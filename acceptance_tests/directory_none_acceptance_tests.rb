#!/usr/bin/env ruby
# frozen_string_literal: true

require 'ttytest'
require './acceptance_tests/startup.rb'

setup_tests(PROMPT_DIRECTORY_NONE)
row = 0
@tty = TTYtest.new_terminal(%(PS1='$ ' ./bin/ncsh), width: 120, height: 20)

startup_tests(row, true)
