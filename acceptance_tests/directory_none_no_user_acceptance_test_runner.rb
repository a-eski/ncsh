#!/usr/bin/env ruby
# frozen_string_literal: true

require './acceptance_tests/tests/startup'
require './acceptance_tests/tests/acceptance_tests'

run_acceptance_tests(PROMPT_DIRECTORY_NONE, PROMPT_SHOW_USER_NONE, false)
