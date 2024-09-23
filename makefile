cc = gcc -std=c99
debug_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -O3 -DNDEBUG
objects = main.o ncsh_commands.o ncsh_terminal.o eskilib/eskilib_string.o ncsh_debug.o ncsh_args.o ncsh_parser.o
target = ncsh

RELEASE ?= 0

ifeq ($(RELEASE), 1)
	cc_with_flags = $(cc) $(release_flags)
else
	cc_with_flags = $(cc) $(debug_flags)
endif

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

main.o : main.c ncsh_commands.h ncsh_terminal.h eskilib/eskilib_string.h eskilib/eskilib_colors.h ncsh_types.h ncsh_parser.h ncsh_args.h
	$(cc_with_flags) -c main.c
ncsh_commands.o : ncsh_commands.h eskilib/eskilib_string.h eskilib/eskilib_colors.h ncsh_types.h ncsh_terminal.h ncsh_args.h
	$(cc_with_flags) -c ncsh_commands.c
ncsh_terminal.o : ncsh_terminal.c ncsh_terminal.h
	$(cc_with_flags) -c ncsh_terminal.c
ncsh_parser.o : ncsh_parser.c ncsh_args.h
	$(cc_with_flags) -c ncsh_parser.c
ncsh_args.o : ncsh_args.c ncsh_args.h
	$(cc_with_flags) -c ncsh_args.c
eskilib_string.o : eskilib_string.c eskilib_string.h
	$(cc_with_flags) -c eskilib/eskilib_string.c
ncsh_debug.o : ncsh_debug.c ncsh_debug.h ncsh_types.h ncsh_args.h
	$(cc_with_flags) -c ncsh_debug.c

clean :
	rm $(target) $(objects)
