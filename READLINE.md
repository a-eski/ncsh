# GNU readline

[Docs](https://tiswww.case.edu/php/chet/readline/readline.html)

## what is the goal of using GNU readline?

1. Support all terminals and OS's that GNU readline supports.
2. Also incorporate ncsh-style autocompletions and tab completions.
3. Allow user to customize and utilize all of readlines functionalities without having to hand roll.
4. Not have to use another library like ncurses, which was slower than readline in experiments.
5. Get rid of current bugs in ncsh 0.0.2 like multiline not working when on the last row of the screen.


## random notes

These are notes of things useful found in readline docs that just didn't belong anywhere else.

* `int rl_editing_mode` can be set to 1 for emacs or 0 for vi mode

* `void rl_replace_line(const char* text, int clear_undo)` can replace the line while preserving rl_point and marks!
Make sure to pass in 0 for clear_undo if you want to maintain the undo list for the current line
`void rl_extend_line_buffer(int len)` can be used to make sure the rl_line_buffer has enough space to hold the replaced line
alternative approach in [Modifying Text](https://tiswww.case.edu/php/chet/readline/readline.html#Modifying-Text)

* `void rl_display_match_list (char **matches, int len, int max)` can be used to display strings like ncsh tab autocomplete currently does.

* `int rl_variable_bind (const char *variable, const char *value)` behaves like inputrc 'set variable value'
Can use `char * rl_variable_value (const char *variable)` to check the value before overriding

* `rl_variable_dumper (int readable)` dump variables, if readable non-zero its in inputrc format

## incorporating ncsh-style autocompletions into GNU readline

### Approaches

#### Approach 1: custom completion

Using custom completion function.

Pros

* Easy to implement

Cons

* Called when tab is pressed, not like how ncsh currently works.

Disqualified due to poor UX, lack of customizability.

#### Approach 2: code changes to readline

Make code changes directly to readline in ncsh_readline form and deploy that as static library.

Pros

* Minimal code changes to ncsh itself, just call ncsh_readline function
* Able to fully change the readline interface
* Able to avoid using static/global variables
* Able to change allocation strategy in readline itself

Cons

* Need to maintain ncsh_readline fork
* Need to vendor ncsh_readline fork
* Need to come with a way to build & install ncsh_readline fork with ncsh

Steps:

* rework rl_getc function to get current best autocompletion
* have reworked rl_getc function save rl_point cursor pos to variable
* have reworked rl_getc function print out that current best autocompletion dimmed
* have reworked rl_getc function reset rl_point to previous value
* rework rl_redisplay?
* call readline to redisplay(rl_redisplay() or rl_forced_update_display())?
* add variable rl_active_autocompletion to represent if autocompletion is pending
* bind right arrow key to complete autocompletion if one is shown

[Readline Variables](https://tiswww.case.edu/php/chet/readline/readline.html#Readline-Variables)
[Character Input](https://tiswww.case.edu/php/chet/readline/readline.html#Character-Input)
[Binding Keys](https://tiswww.case.edu/php/chet/readline/readline.html#Binding-Keys)

#### Approach 3: overriding readline using function pointers

Override readline functionality using the exposed function pointers it provides.

Pros

* Don't need to vendor or make code changes to readline itself.
* Custom functionality without having to deal with readline itself.
* Can always incorporate into readline fork itself later if necessary

Cons

* Some readline code duplicated into ncsh itself
* Not as flexible
* Need to figure out how to pass in variables to readline around autocomplete
* Need to figure out how to pass in variables to readline around allocations

Steps:

* replace rl_getc function to get current best autocompletion
* have replaced rl_getc function save rl_point cursor pos to variable
* have replaced rl_getc function print out that current best autocompletion dimmed
* have replaced rl_getc function reset rl_point to previous value
* replace rl_redisplay?
* call readline to redisplay (rl_redisplay() or rl_forced_update_display())?
* add variable rl_active_autocompletion to represent if autocompletion is pending
* bind right arrow key to complete autocompletion if one is shown

If replacing rl_getc, use rl_getc_function to override
If replacing rl_redisplay use rl_redisplay_function to override
Need to utilize keymaps when binding right arrow key, or rl_bind_key, not 100% sure

[Readline Variables](https://tiswww.case.edu/php/chet/readline/readline.html#Readline-Variables)
[Character input](https://tiswww.case.edu/php/chet/readline/readline.html#Character-Input)
[Keymaps](https://tiswww.case.edu/php/chet/readline/readline.html#Keymaps)

#### Approach 4: using the readline alternate interface

[Alternate Interface](https://tiswww.case.edu/php/chet/readline/readline.html#Alternate-Interface)
