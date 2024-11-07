# ncsh
An interactive unix shell focused on speed and building everything from the ground up.

# to build from source
make
sudo make install

# notes
sudo make install ONLY calls the install command to add the ELF executable to DESTDIR /usr/local.
Install directory currently only supports /usr/bin. Will work on supporting any DESTDIR via makefile in the future.
History file .ncsh_history tries to use XDG_CONFIG_HOME if available, or else HOME.

# goals
1. autocomplete
2. piping, output/input redirection, !, &&, ||, etc.
3. up/down arrow keys to navigate through history - DONE :)
4. built in commands like ls, cd, q/exit/quit/Ctrl+D, history, export, kill, setting environment variables, etc.
5. aliasing
6. better prompt line
7. add non-iteractive mode
8. incoporate z-oxide or z-oxide like cd command
