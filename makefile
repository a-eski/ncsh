cc = gcc -std=c99 -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined
objects = main.o shl_commands.o shl_terminal.o shl_string.o
target = shl

$(target) : $(objects)
	$(cc) -o $(target) $(objects)

main.o : main.c shl_commands.h shl_terminal.h shl_string.h shl_output.h shl_types.h
	$(cc) -c main.c
shl_commands.o : shl_commands.h shl_string.h shl_output.h shl_types.h
	$(cc) -c shl_commands.c
shl_terminal.o : shl_terminal.c shl_terminal.h
	$(cc) -c shl_terminal.c
shl_string.o : shl_string.c shl_string.h
	$(cc) -c shl_string.c

clean :
	rm $(target) $(objects)
