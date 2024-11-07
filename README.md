# ncsh
An interactive unix shell focused on speed, handy features, and building everything from the ground up.

# to build from source
make<br />
sudo make install

# notes
- sudo make install ONLY calls the install command to add the ELF executable to /usr/local.
- Install directory is by default /usr/local. Pass in DESTDIR if you want to install it somewhere else
- History file .ncsh_history tries to use XDG_CONFIG_HOME if available, or else HOME.

# goals
1. be able to install the shell and use it - DONE :)
2. autocomplete - DONE :)
3. weighted autocomplete
4. basic vm that supports multiple pipes - DONE :)
5. vm that supports piping, output/input redirection, !, &&, ||, etc.
6. up/down arrow keys to navigate through history - DONE :)
7. support removing entries from history and autocompletions
8. built in commands like ls, cd, q/exit/quit/Ctrl+D, history - DONE :)
9. move built-in commands like export, kill, setting environment variables, etc.
10. aliasing for user-defined commands
11. better prompt line
12. add non-iteractive mode
13. incoporate z/autojump/z-oxide like cd command
14. exclude features at compilation time in case users want lightweight shell without autocompletion or z like cd command
