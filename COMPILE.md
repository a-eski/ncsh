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

### Testing Local

``` sh
# run CI tests
make check

# run all tests locally
make l

# run tests individually
make test_autocompletions
make test_history
make test_parser
make test_fzf
make test_z
make integration_tests
```

### Fuzz Testing

``` sh
make fuzz_autocompletions
make fuzz_history
make fuzz_parser
```
