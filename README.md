# ncsh

An interactive unix shell focused on speed and building everything from scratch.

## features

* Autocompletions as you type: autocompletions based on history and weight.
* Tab autocompletions: view all available options by pressing tab and cycle through them with up/down keys and select with enter.
* History: command history tracked and can be cycled through using up/down keys.
* Manipulate input: home, end, CTRL+W to delete a word, CTRL+U to delete a line, etc.

## dependencies

none, asides from the C standard library and POSIX extensions.

## to build from source

``` sh
make

# or can use cmake
cmake build -S ./ -B ./bin
cd bin
make
```

## to install

``` sh
sudo make install

# or can use cmake
cd bin
sudo make install
```

## installation notes

* sudo make install ONLY calls the install command to add the ELF executable to /usr/local.
* Install directory is by default /usr/local.
* Pass in DESTDIR if you want to install it somewhere else.
* History file .ncsh_history tries to use XDG_CONFIG_HOME if available, or else HOME.

## goals

* rc file and more configurations
* more compile-time configurations
* aliasing
* frecency-based autocomplete (currently is a simple weight of how many times it has been used).
* support removing entries from history and autocompletions (use sqlite??)
* move built-in commands like export, kill, setting environment variables, etc.
* aliasing for user-defined commands, user configuration
* better prompt line and configurable prompt line
* support scripts through a non-iteractive mode
* build for and test on mac (I don't have one)
* build for and test on windows with msys2
* math
