# ttyterm

A lightweight wrapper library for unibilium.

Useful for writing CLI/REPL/shell applications.

Still in very very early stages of development.

## Features

* Easy access to terminal capabilities.
* Simple output interfaces.
* Tracks cursor position, terminal size, and saved cursor position automatically.
* Falls back to ASCII control characters when can't load capabilities.
* Advanced capabilities with multiple fallbacks.

## Unibilium

Props to Neovim maintainers and [unibilium](https://github.com/neovim/unibilium/tree/master).

## Example

Example is in main.c

## Code Definitions

* scr: screen
* clr: clear
* eos: end of screen
* eol: end of line
* bol: beginning of line
