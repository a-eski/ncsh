cc = gcc -std=c99 -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined
objects = main.o ncsh_commands.o ncsh_terminal.o ncsh_string.o
target = ncsh

$(target) : $(objects)
	$(cc) -o $(target) $(objects)

main.o : main.c ncsh_commands.h ncsh_terminal.h ncsh_string.h ncsh_output.h ncsh_types.h
	$(cc) -c main.c
ncsh_commands.o : ncsh_commands.h ncsh_string.h ncsh_output.h ncsh_types.h
	$(cc) -c ncsh_commands.c
ncsh_terminal.o : ncsh_terminal.c ncsh_terminal.h
	$(cc) -c ncsh_terminal.c
ncsh_string.o : ncsh_string.c ncsh_string.h
	$(cc) -c ncsh_string.c

clean :
	rm $(target) $(objects)
