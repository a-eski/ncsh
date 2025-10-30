# frozen_string_literal: true

# acceptance_tests.rb: the main acceptance tests for ncsh.

require './acceptance_tests/tests/common'
require './acceptance_tests/tests/basic'
require './acceptance_tests/tests/builtins'
require './acceptance_tests/tests/expansion'
require './acceptance_tests/tests/autocompletion'
require './acceptance_tests/tests/history'
require './acceptance_tests/tests/logic'
require './acceptance_tests/tests/if'
require './acceptance_tests/tests/keys'
require './acceptance_tests/tests/for'
require './acceptance_tests/tests/pipes'
require './acceptance_tests/tests/redirection'
require './acceptance_tests/tests/startup'
require './acceptance_tests/tests/syntax'
require './acceptance_tests/tests/variables'
require './acceptance_tests/tests/z'

def run_acceptance_tests(prompt_directory_option, prompt_user_option, is_custom_prompt)
  setup_tests(prompt_directory_option, prompt_user_option, is_custom_prompt)

  row = 0
  @tty = TTYtest.new_terminal(%(./bin/ncsh), width: 180, height: 160, use_return_for_newline: true)
  row = startup_tests(row, true)
  row = basic_tests(row)
  row = keys_tests(row)
  row = pipe_tests(row)
  row = history_tests(row)
  row = stdout_redirection_tests(row)
  row = z_tests(row)
  row = stdin_redirection_tests(row)
  builtins_tests(row)
  @tty.send_line(%(exit))

  row = 0
  @tty = TTYtest.new_terminal(%(./bin/ncsh), width: 180, height: 160, use_return_for_newline: true)
  row = startup_tests(row, false)
  # row = autocompletion_tests(row)
  row = syntax_tests(row)
  row = expansion_tests(row)
  variables_tests(row)
  @tty.send_line(%(exit))

  row = 0
  @tty = TTYtest.new_terminal(%(./bin/ncsh), width: 180, height: 160, use_return_for_newline: true)
  row = startup_tests(row, false)
  row = logic_tests(row)
  if_tests(row)

  row = 0
  @tty = TTYtest.new_terminal(%(./bin/ncsh), width: 180, height: 160, use_return_for_newline: true)
  row = startup_tests(row, false)
  for_tests(row)
  @tty.send_line(%(exit))
  # row = paste_tests(row)
  # row = multiline_tests(row)
end
