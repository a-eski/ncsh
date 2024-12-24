# ncsh

An interactive unix shell focused on speed and building everything from the ground up.

## dependencies

none, asides from the C standard library and POSIX extensions.

## to build from source

make

## to install

sudo make install

## installation notes

* sudo make install ONLY calls the install command to add the ELF executable to /usr/local.
* Install directory is by default /usr/local.
* Pass in DESTDIR if you want to install it somewhere else.
* History file .ncsh_history tries to use XDG_CONFIG_HOME if available, or else HOME.

## goals

* be able to install the shell and use it - DONE :)
* autocomplete - DONE :)
* weighted autocomplete - DONE :)
* frecency-based autocomplete
* basic vm that supports multiple pipes - DONE :)
* vm that supports piping, output/input redirection - DONE :)
* vm that supports !, &&, ||, etc.
* up/down arrow keys to navigate through history - DONE :)
* support removing entries from history and autocompletions
* built in commands like ls, cd, q/exit/quit/Ctrl+D, history - DONE :)
* move built-in commands like export, kill, setting environment variables, etc.
* aliasing for user-defined commands, user configuration
* better prompt line - DONE (for now) :)
* add non-iteractive mode
* incoporate z/autojump/z-oxide like cd command - DONE :)
* globs (aka wildcard expansion)! - DONE :)
