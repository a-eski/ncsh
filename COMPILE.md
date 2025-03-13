# ncsh Compilation, Debugging, and Profiling Notes

## Compile

### Notes

I tried to have the makefile follow conventions as closely as possible. CC, CFLAGS, STD, DESTDIR, BUILDDIR, RELEASE can all be passed in from command line with make. Other standard options will eventually be added, but not all are supported at time of writing.

There are also shorthand versions of the commands to make it easier to iterate. For example, you can run `make test_readline` with `make tr`. If you are running ncsh as your shell, it has a builtin alias for `make`, `m`. So you can run `m tr` instead of `make test_readline`. This comes in handy when running tests over and over and making lots of changes.

### Compiling with clang

You can build any of the builds listed below with clang if you have a version of clang installed which supports C23 and constexpr. Simply add `CC=clang` to the end of any of the build commands.

``` sh
sudo apt install clang clang-19 clang-tools-19

# on ubuntu/debian, you may have to use clang-19
make release CC=clang
make release CC=clang-19
```

### Compile Debug Builds

``` sh
# debug build
make debug
make d # equivalent to make debug

# debug unity build
make unity_debug
make ud # equivalent to make unity_debug

# if running ncsh as your shell, you can use:
m d # equivalent to make debug
m ud #equivalent to make unity_debug
```

### Compile Release Builds

``` sh
# standard build (all 3 are equivalent)
make
make release
make r

# unity build (both are equivalent)
make unity
make u
```

## Debug

These are just notes so I don't have to lookup these all of the time.

### GDB

``` sh
gdb -tui ./bin/ncsh
# c - continue
# s - step
# n - step over
```

### Valgrind

``` sh
valgrind --leak-check=yes ./bin/ncsh
valgrind --leak-check=full ./bin/ncsh
valgrind --track-fds=yes ./bin/ncsh
valgrind --leak-check=yes --track-fds=yes ./bin/ncsh
```

## Profiling

### gprof

1. compile with -pg, -O3 and -DNDEBUG
2. run program
3. generate report: gprof gmon.out > analysis.txt
4. review analysis.txt

## Testing

There are shorthands for all of the commands in the makefile.

### Dependencies

There are unit tests and acceptance tests. You can run a subset of unit tests with `make check`. You can run all tests with `make check_local` aka `make l`, which include the acceptance tests. However, to run acceptance tests you need to install ruby and [ttytest2](https://github.com/a-eski/ttytest2).

Example for Ubuntu/Debian:

``` sh
sudo apt-get install ruby-full
sudo gem install ttytest2
```

### Testing Local

``` sh
# run CI tests (not all tests)
make check # or make c

# run all tests
make check_local # or make l

# run tests individually
make test_autocompletions # or make ta
make test_history # or make th
make test_parser # or make tp
make test_fzf # or make tf
make test_z # or make tz
make acceptance_tests # or make at
```

### Fuzz Testing

``` sh
make fuzz_autocompletions # of make fa
make fuzz_history # or make fh
make fuzz_parser # or make fp
make fuzz_z # or make fz
make fuzz_z_add # or make fza
```
