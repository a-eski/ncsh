# ncsh

An interactive unix shell focused on speed and building everything from scratch.

## features

* Autocompletions as you type: autocompletions based on history and weight.
* Tab autocompletions: view available options by pressing tab and cycle through with up/down keys and select with enter.
* History: command history tracked and can be cycled through using up/down keys.
* Manipulate input: home, end, CTRL+W to delete word, CTRL+U to delete line, etc.
* z: a native autojump/z-oxide/z command builtin.

### z? What is z?

The z command is a better cd command.

It uses your history and fuzzy matching to determine which directories you want to navigate to.

#### Example

You want to go to '/home/your-user/repos/personal-repos/ncsh', a directory you have already been to before:

``` sh
# Instead of typing:
cd ~/repos/personal-repos/ncsh

# you can just type
z ncsh
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

## dependencies

Using 256 color terminal is recommended.

For code compilation and installation you don't need to install anything, ncsh only uses the C standard library and POSIX extensions.

## to build from source

For more details on compilation, see COMPILE.md

``` sh
make

# you can also build with clang. Requires clang 19+.
make CC=clang

# on ubuntu/debian, you may have to install and use clang-19 explicitly.
make CC=clang-19
```

## installing

``` sh
# install after building
sudo make install

# set as your shell
sudo usermod -s /bin/ncsh <username>
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
* custom prompt colors, backgrounds
* support scripts through a non-iteractive mode
* build for and test on mac (I don't have one)?
* background jobs support
* math
* better posix support
* add to path (works but has some issues when running ncsh as main shell)
* variables
