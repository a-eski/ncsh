STD = -std=c2x
CC ?= gcc
DESTDIR ?= /bin
RELEASE ?= 0
# debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -fstack-protector-all -fsanitize=address,undefined,leak -g
# debug_flags = -Wall -Wextra -Werror -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
debug_flags= -pedantic -Wall -Wextra -pedantic-errors -Wundef -Wshadow -Wbad-function-cast -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -O2 -pipe -fstack-protector-all -fsanitize=address,undefined -g
LDFLAGS=-fsanitize=address -fsanitize=undefined
# -DNCSH_DEBUG
# release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -O3 -DNDEBUG
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -flto -O3 -ffast-math -march=native -DNDEBUG
# fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wwrite-strings -fsanitize=address,leak,fuzzer -DNDEBUG -g
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/ttyterm.o obj/tcaps.o obj/unibilium.o obj/uninames.o obj/uniutil.o
target = u

ifeq ($(CC), gcc)
	release_flags += -s
endif

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
endif

ifneq ($(OS),Windows_NT)
  	TERMINFO="$(shell ncursesw6-config --terminfo 2>/dev/null || \
                         ncurses6-config  --terminfo 2>/dev/null || \
                         ncursesw5-config --terminfo 2>/dev/null || \
                         ncurses5-config  --terminfo 2>/dev/null || \
                         echo "/usr/share/terminfo")"
  	TERMINFO_DIRS="$(shell ncursesw6-config --terminfo-dirs 2>/dev/null || \
                         ncurses6-config  --terminfo-dirs 2>/dev/null || \
                         ncursesw5-config --terminfo-dirs 2>/dev/null || \
                         ncurses5-config  --terminfo-dirs 2>/dev/null || \
                         echo "/etc/terminfo:/lib/terminfo:/usr/share/terminfo:/usr/lib/terminfo:/usr/local/share/terminfo:/usr/local/lib/terminfo")"
else
  	TERMINFO_DIRS=""
  	TERMINFO=""
endif
TTYTERM_DEFINES ?= -DTERMINFO='$(TERMINFO)' -DTERMINFO_DIRS='$(TERMINFO_DIRS)'

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/%.o: lib/%.c
	$(cc_with_flags) $(TTYTERM_DEFINES) -c $< -o $@

obj/%.o: %.c
	$(cc_with_flags) -c $< -o $@

# Release build
release:
	make RELEASE=1

# Debug build
debug :
	make -B RELEASE=0

# Format the project
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
cf :
	make clang_format

# Perform static analysis on the project
.PHONY: scan_build
scan_build:
	scan-build-19 -analyze-headers make

# Clean-up
.PHONY: clean
clean :
	rm $(target) $(objects)
