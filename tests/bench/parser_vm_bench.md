# shell bench: parser and vm

## Before

Was high 50's of 60's for all

## after parser and minor vm optimizations

hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
Benchmark 1: ls
  Time (mean ± σ):      54.9 ms ±   0.5 ms    [User: 3.9 ms, System: 1.0 ms]
  Range (min … max):    54.1 ms …  56.4 ms    54 runs

Benchmark 2: ls | sort
  Time (mean ± σ):      57.4 ms ±   1.5 ms    [User: 3.3 ms, System: 1.8 ms]
  Range (min … max):    55.5 ms …  62.4 ms    49 runs

Benchmark 3: ls > t.txt
  Time (mean ± σ):      55.6 ms ±   0.5 ms    [User: 3.0 ms, System: 1.9 ms]
  Range (min … max):    54.6 ms …  56.7 ms    53 runs

Benchmark 4: ls | sort | wc -c
  Time (mean ± σ):      55.2 ms ±   0.7 ms    [User: 3.2 ms, System: 1.8 ms]
  Range (min … max):    54.1 ms …  57.1 ms    54 runs

Benchmark 5: ls | sort | wc -c > t2.txt
  Time (mean ± σ):      56.3 ms ±   0.6 ms    [User: 3.4 ms, System: 1.7 ms]
  Range (min … max):    55.4 ms …  57.9 ms    53 runs

Summary
  ls ran
    1.01 ± 0.02 times faster than ls | sort | wc -c
    1.01 ± 0.01 times faster than ls > t.txt
    1.03 ± 0.01 times faster than ls | sort | wc -c > t2.txt
    1.05 ± 0.03 times faster than ls | sort

hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
Benchmark 1: ls
  Time (mean ± σ):      55.3 ms ±   0.5 ms    [User: 3.6 ms, System: 1.3 ms]
  Range (min … max):    54.4 ms …  56.5 ms    54 runs

Benchmark 2: ls | sort
  Time (mean ± σ):      55.2 ms ±   0.6 ms    [User: 3.4 ms, System: 1.5 ms]
  Range (min … max):    53.8 ms …  56.2 ms    55 runs

Benchmark 3: ls > t.txt
  Time (mean ± σ):      54.9 ms ±   0.5 ms    [User: 3.3 ms, System: 1.6 ms]
  Range (min … max):    53.9 ms …  56.2 ms    53 runs

Benchmark 4: ls | sort | wc -c
  Time (mean ± σ):      57.0 ms ±   0.6 ms    [User: 3.6 ms, System: 1.5 ms]
  Range (min … max):    56.0 ms …  58.5 ms    52 runs

Benchmark 5: ls | sort | wc -c > t2.txt
  Time (mean ± σ):      56.9 ms ±   0.5 ms    [User: 3.2 ms, System: 1.9 ms]
  Range (min … max):    55.6 ms …  58.2 ms    51 runs

Summary
  ls > t.txt ran
    1.01 ± 0.01 times faster than ls | sort
    1.01 ± 0.01 times faster than ls
    1.04 ± 0.01 times faster than ls | sort | wc -c > t2.txt
    1.04 ± 0.02 times faster than ls | sort | wc -c

## Continue VM optimiztions

hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
Benchmark 1: ls
  Time (mean ± σ):      54.6 ms ±   0.5 ms    [User: 2.9 ms, System: 2.0 ms]
  Range (min … max):    53.4 ms …  55.6 ms    53 runs

Benchmark 2: ls | sort
  Time (mean ± σ):      56.6 ms ±   0.8 ms    [User: 3.5 ms, System: 1.5 ms]
  Range (min … max):    55.1 ms …  58.5 ms    52 runs

Benchmark 3: ls > t.txt
  Time (mean ± σ):      55.6 ms ±   1.7 ms    [User: 3.8 ms, System: 1.4 ms]
  Range (min … max):    53.4 ms …  60.8 ms    55 runs

Benchmark 4: ls | sort | wc -c
  Time (mean ± σ):      57.3 ms ±   1.2 ms    [User: 3.1 ms, System: 2.0 ms]
  Range (min … max):    55.8 ms …  61.1 ms    51 runs

Benchmark 5: ls | sort | wc -c > t2.txt
  Time (mean ± σ):      56.9 ms ±   1.0 ms    [User: 3.3 ms, System: 1.9 ms]
  Range (min … max):    55.3 ms …  60.8 ms    52 runs

Summary
  ls ran
    1.02 ± 0.03 times faster than ls > t.txt
    1.04 ± 0.02 times faster than ls | sort
    1.04 ± 0.02 times faster than ls | sort | wc -c > t2.txt
    1.05 ± 0.02 times faster than ls | sort | wc -c

hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
Benchmark 1: ls
  Time (mean ± σ):      55.7 ms ±   0.5 ms    [User: 3.1 ms, System: 1.8 ms]
  Range (min … max):    54.7 ms …  56.8 ms    53 runs

Benchmark 2: ls | sort
  Time (mean ± σ):      54.9 ms ±   1.6 ms    [User: 3.1 ms, System: 1.9 ms]
  Range (min … max):    52.8 ms …  58.2 ms    53 runs

Benchmark 3: ls > t.txt
  Time (mean ± σ):      54.8 ms ±   0.7 ms    [User: 3.0 ms, System: 1.9 ms]
  Range (min … max):    53.8 ms …  57.2 ms    54 runs

Benchmark 4: ls | sort | wc -c
  Time (mean ± σ):      56.7 ms ±   0.6 ms    [User: 3.4 ms, System: 1.7 ms]
  Range (min … max):    55.7 ms …  58.4 ms    51 runs

Benchmark 5: ls | sort | wc -c > t2.txt
  Time (mean ± σ):      57.8 ms ±   1.7 ms    [User: 3.5 ms, System: 1.6 ms]
  Range (min … max):    56.0 ms …  64.2 ms    53 runs

Summary
  ls > t.txt ran
    1.00 ± 0.03 times faster than ls | sort
    1.02 ± 0.02 times faster than ls
    1.04 ± 0.02 times faster than ls | sort | wc -c
    1.06 ± 0.03 times faster than ls | sort | wc -c > t2.txt
