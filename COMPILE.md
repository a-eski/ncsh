# ncsh Compilation, Debugging, Profiling, and Benchmarking Notes

## Compile

### Compile Debug Builds

``` sh
make debug
make d # equivalent to make debug
make debug CC=clang
make d CC=clang # equivalent to make debug CC=clang
```

### Compile Release Builds

``` sh
make
make CC=clang

# cmake
cmake build -S ./ -B bin/
cd bin
make
```

## Debug

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
make check

# run all tests
make check_local

# run tests individually
make test_autocompletions
make test_history
make test_parser
make test_fzf
make test_z
make acceptance_tests
```

### Fuzz Testing

``` sh
make fuzz_autocompletions
make fuzz_history
make fuzz_parser
./src/z/z_fuzz.sh
./src/z/z_add_fuzz.sh
```
