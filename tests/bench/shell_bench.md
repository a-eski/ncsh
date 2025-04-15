# shell benchmarks

make bpv

## Before

Benchmark 1: ls
  Time (mean ± σ):      58.3 ms ±   0.6 ms    [User: 3.1 ms, System: 2.0 ms]
  Range (min … max):    56.7 ms …  59.4 ms    52 runs

Benchmark 2: ls | sort
  Time (mean ± σ):      58.8 ms ±   0.5 ms    [User: 2.8 ms, System: 2.2 ms]
  Range (min … max):    57.9 ms …  59.8 ms    50 runs

Benchmark 3: ls > t.txt
  Time (mean ± σ):      63.4 ms ±   4.5 ms    [User: 3.3 ms, System: 2.2 ms]
  Range (min … max):    58.8 ms …  75.5 ms    40 runs

Benchmark 4: ls | sort | wc -c
  Time (mean ± σ):      60.3 ms ±   0.8 ms    [User: 3.5 ms, System: 1.7 ms]
  Range (min … max):    58.5 ms …  62.1 ms    49 runs

Benchmark 5: ls | sort | wc -c > t2.txt
  Time (mean ± σ):      60.1 ms ±   0.7 ms    [User: 3.5 ms, System: 1.8 ms]
  Range (min … max):    58.5 ms …  62.2 ms    48 runs

Summary
  ls ran
    1.01 ± 0.01 times faster than ls | sort
    1.03 ± 0.02 times faster than ls | sort | wc -c > t2.txt
    1.03 ± 0.02 times faster than ls | sort | wc -c
    1.09 ± 0.08 times faster than ls > t.txt

## After (struct Command)
