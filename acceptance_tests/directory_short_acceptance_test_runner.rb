#!/usr/bin/env ruby
# frozen_string_literal: true

require './acceptance_tests/tests/startup'
require './acceptance_tests/tests/acceptance_tests'

run_acceptance_tests(PROMPT_DIRECTORY_SHORT, PROMPT_SHOW_USER_NORMAL, false)
