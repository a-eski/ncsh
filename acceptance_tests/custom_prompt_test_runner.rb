#!/usr/bin/env ruby
# frozen_string_literal: true

require './acceptance_tests/tests/startup'
require './acceptance_tests/tests/acceptance_tests'

# the custom prompt is set from compilation '-DNCSH_PROMPT_ENDING_STRING=>' in ../tests_at.sh
# the custom prompt is '>'
run_acceptance_tests(PROMPT_DIRECTORY_NONE, PROMPT_SHOW_USER_NONE, true)
