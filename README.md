# ncsh

An interactive unix shell focused on speed and building everything from scratch.

## features

* Autocompletions as you type: autocompletions based on history and weight.
* Tab autocompletions: view all available options by pressing tab and cycle through them with up/down keys and select with enter.
* History: command history tracked and can be cycled through using up/down keys.
* Manipulate input: home, end, CTRL+W to delete a word, CTRL+U to delete a line, etc.

## dependencies

Your terminal emulator must support at least 16 colors for autocomplete to work as expected (autocompletion suggestions are 'dimmed').

You don't need to install anything, ncsh only uses the C standard library and POSIX extensions.

## to build from source

For more details on compilation, see COMPILE.md

``` sh
make

# you can also build with clang.
make CC=clang
```

## installing

``` sh
sudo make install

# or can use cmake
cd bin
sudo make install

# set as your shell
usermod -s /bin/ncsh <username>
```

## installation notes

* sudo make install ONLY calls the install command to add the ELF executable to /usr/local.
* Install directory is by default /usr/local.
* Pass in DESTDIR if you want to install it somewhere else.
* History file .ncsh_history tries to use XDG_CONFIG_HOME if available, or else HOME.

## running tests

Please see COMPILE.md.

## reading code

There are some high-level comments in the header files, with more detailed cod documentation in the source files.

## goals

* rc file configurations
* more compile-time configurations
* frecency-based autocomplete
* move built-in commands like export, setting environment variables, etc.
* aliasing for user-defined commands
* custom prompt line layout, colors, backgrounds
* support scripts through a non-iteractive mode
* build for and test on mac (I don't have one)?
* build for and test on windows with msys2?
* background jobs support
* math
* better posix support
* add to path
* variables
