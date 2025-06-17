# ncsh

An interactive unix shell focused on speed and building everything from scratch.

The goal is to build a shell with modern features and experience and as close to POSIX compliance as possible.

## Features

* Autocompletions as you type: autocompletions based on history and weight.
* Tab shows all autocompletions: view available options by pressing tab and cycle through with up/down keys and select with enter.
* History: command history tracked and can be cycled through using up/down keys.
* Manipulate input: home, end, CTRL+W to delete word, CTRL+U to delete line, etc.
* z: a native autojump/z-oxide/z command builtin.
* Working towards POSIX compliance.

### Controls

A lot of controls will feel familar to GNU readline applications like Bash.

* Right/left arrow keys move the cursor
* Up/down arrow keys move through your history
* home/end go to start and end of the line
* Ctrl+W deletes a word, Ctrl+U deletes the entire line
* If an autocompletion is visible and you are at the end of the line, right arrow will complete the autocompletion. Will add functionality to be able to switch this to tab
* Backspace and delete work as expected.
* Tab will show you all possible completions for your current input. Use up and down arrow to scroll through, then enter to select the completion.

### History

History uses the same format as bash, text separated by newlines:

``` txt
ls -a --color
nvim .
gcc main.c -o prog
```

You can add entries to history or remove entries from history. Typing 'history' displays the history.

If spaces included, surround with any type of quotes.

``` sh
# add entry to history.
history add "ls -a --color"

# remove entry from history. Removes duplicates from history then removes the entry specified.
history rm vim
history remove 'vim .'
```

You can also clean the history (remove all duplicates), or get the current history count.

``` sh
# get number of history entries
history count

# remove all duplicates from history
history clean
```

### z? What is z?

The z command is a better cd command.

It uses your history and fuzzy matching to determine which directories you want to navigate to.

z maintains a 'z database' in a binary file in your ncsh config location. It maintains a score and last accessed time for all your files.

z tries to find the directory you entered after the z command using fuzzy finding with an fzf native implementation.

z combines the fzf score with a z score (how often and how recently you accessed that directory) to determine which directory to send you to.

If it takes you to the wrong directory, you can run the command again to go to your second highest scored directory matching that input.

z also searches for local directory matches, so you can use it like cd as well.

#### Examples

You want to go to a directory you have been to using z:

``` sh
# Instead of typing:
cd ~/repos/personal-repos/ncsh

# you can just type
z ncsh
```

When starting it is useful to add things to your z database:

``` sh
z add ~/repos/personal-repos/ncsh
z add /usr/bin
z add ~/.config

# You can now visit those directories like this:
z ncsh
z usr
z conf
```

You can remove entries from your z database if they are annoying you:

``` sh
z remove ~/ncsh
z rm /usr/bin
z rm ~/.config
```

To see what is in your z database, use the 'z print' command:

``` sh
z print
```

#### z fzf?

z utilizes a native implementation of fzf based on [telescope-fzf-native.nvim](https://github.com/nvim-telescope/telescope-fzf-native.nvim).

This means, in addition to z being able to help you navigate better, you can also utilize [fzf syntax](https://github.com/junegunn/fzf#search-syntax):

| Token     | Match type                 | Description                          |
| --------- | -------------------------- | ------------------------------------ |
| `sbtrkt`  | fuzzy-match                | Items that match `sbtrkt`            |
| `'wild`   | exact-match (quoted)       | Items that include `wild`            |
| `^music`  | prefix-exact-match         | Items that start with `music`        |
| `.mp3$`   | suffix-exact-match         | Items that end with `.mp3`           |
| `!fire`   | inverse-exact-match        | Items that do not include `fire`     |
| `!^music` | inverse-prefix-exact-match | Items that do not start with `music` |
| `!.mp3$`  | inverse-suffix-exact-match | Items that do not end with `.mp3`    |

## Dependencies

Using 256 color terminal is recommended.

For code compilation and installation you don't need to install anything, ncsh only uses the C standard library and POSIX extensions.

Developed on Debian, so hopefully there are not issues with GLIBC, but please submit any linking issues found.

## Building from source

For more details on compilation, see COMPILE.md

``` sh
make

# you can also build with clang. Requires clang 19+.
make CC=clang

# on ubuntu/debian, you may have to install and use clang-19 explicitly.
make CC=clang-19
```

## Installation

``` sh
# install after building
sudo make install

# set as your shell
# note: not fully posix compliant so you will likely have issues
#   for example, gdb does not work with ncsh as main shell
#   you can set as your startup program instead
sudo usermod -s /bin/ncsh <username>
```

### Installation Notes

* sudo make install ONLY calls the install command to add the ELF executable to /usr/local.
* Install directory is by default /usr/local.
* Pass in DESTDIR if you want to install it somewhere else.
* History file .ncsh_history, .ncshrc, and z database files try to use XDG_CONFIG_HOME if available, or else use environment variable HOME.

## Running tests

Please see COMPILE.md.

## Goals

### Short-term

* Have .ncshrc utilize builtins instead of DSL like it currently does
* Finish set/unset, enable/disable, export implementations

### Long-term

* Incorporate traditional tab autocompletions
* Adjust current implementation of tab 'show all' autocompletions
* More rc file configurations
* More compile-time configurations
* Frecency-based autocomplete
* More built-in commands like export, setting environment variables, etc.
* Custom prompt colors, backgrounds
* Improve noninteractive mode
* Add command-line flags for noninteractive mode
* Background jobs implementation
* Math/Arithmetic implementation
* Better POSIX compliance
* Usable with GDB (currently doesn't work)
