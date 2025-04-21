# Autocompletions benchmarks

## autocompletions_tests

### before

Time (mean ± σ):       5.5 ms ±   0.5 ms    [User: 2.5ms, System: 1.2ms]
Range (min … max):     4.6 ms …   13.9 ms   564 runs

### search optimization 1

Time (mean ± σ):       5.2 ms ±   0.4 ms    [User: 2.4ms, System: 1.1ms]
Range (min … max):     4.6 ms …   7.5 ms    402 runs

### search & match optimization 1

Time (mean ± σ):       5.0 ms ±   0.4 ms    [User: 2.3 ms, System: 1.1 ms]
Range (min … max):     4.4 ms …   7.3 ms    564 runs

### match optimization 2

Time (mean ± σ):       4.8 ms ±   0.3 ms    [User: 2.4 ms, System: 0.9 ms]
Range (min … max):     4.4 ms …   7.3 ms    595 runs

Time (mean ± σ):       4.9 ms ±   0.3 ms    [User: 2.2 ms, System: 1.0 ms]
Range (min … max):     4.4 ms …   7.3 ms    438 runs

## autocompletions_bench

### search optimization 1

Time (mean ± σ):       4.6 ms ±   0.4 ms    [User: 2.4 ms, System: 0.9 ms]
Range (min … max):     4.1 ms …   7.1 ms    714 runs

### match optimization 1

Time (mean ± σ):       4.4 ms ±   0.4 ms    [User: 2.2ms, System: 0.7ms]
Range (min … max):     3.8 ms …   6.3 ms    641 runs

Time (mean ± σ):       4.3 ms ±   0.3 ms    [User: 2.2 ms, System: 0.6 ms]
Range (min … max):     3.9 ms …   6.1 ms    630 runs

### bench: match optimization 2

Time (mean ± σ):       4.2 ms ±   0.3 ms    [User: 2.2 ms, System: 0.6 ms]
Range (min … max):     3.9 ms …   7.7 ms    734 runs

Time (mean ± σ):       4.3 ms ±   0.3 ms    [User: 2.2 ms, System: 0.7 ms]
Range (min … max):     3.8 ms …   6.4 ms    734 runs

### bench: rework find not to use recursion

Time (mean ± σ):       4.3 ms ±   0.3 ms    [User: 2.2 ms, System: 0.6 ms]
Range (min … max):     3.8 ms …   6.1 ms    753 runs

Time (mean ± σ):       4.2 ms ±   0.2 ms    [User: 2.1 ms, System: 0.5 ms]
Range (min … max):     3.8 ms …   5.6 ms    733 runs

Time (mean ± σ):       4.2 ms ±   0.3 ms    [User: 2.2 ms, System: 0.6 ms]
Range (min … max):     3.8 ms …   5.6 ms    713 runs

### bench: completely rework ac_match

Time (mean ± σ):       4.2 ms ±   0.3 ms    [User: 2.1 ms, System: 0.6 ms]
Range (min … max):     3.7 ms …   6.4 ms    735 runs

Time (mean ± σ):       4.1 ms ±   0.3 ms    [User: 2.1 ms, System: 0.6 ms]
Range (min … max):     3.7 ms …   7.4 ms    663 runs

## release bench

### bench release: before

Time (mean ± σ):     549.8 µs ±  61.8 µs    [User: 6.8 µs, System: 9.9 µs]
Range (min … max):   498.2 µs … 1261.8 µs    5484 runs

Time (mean ± σ):     551.2 µs ±  61.5 µs    [User: 6.6 µs, System: 12.0 µs]
Range (min … max):   497.5 µs … 1154.0 µs    5455 runs

### bench release: rework find to not use recursion

Time (mean ± σ):     544.3 µs ±  52.4 µs    [User: 5.9 µs, System: 9.3 µs]
Range (min … max):   501.6 µs … 1042.2 µs    5303 runs

Time (mean ± σ):     543.3 µs ±  57.6 µs    [User: 4.9 µs, System: 10.5 µs]
Range (min … max):   474.1 µs … 1302.1 µs    5393 runs

Time (mean ± σ):     541.2 µs ±  51.3 µs    [User: 4.5 µs, System: 10.7 µs]
Range (min … max):   500.1 µs … 1072.7 µs    5327 runs

### bench release: completely rework ac_match

Time (mean ± σ):     535.6 µs ±  60.6 µs    [User: 5.4 µs, System: 10.3 µs]
Range (min … max):   491.4 µs … 1231.5 µs    3766 runs

Time (mean ± σ):     539.6 µs ±  56.3 µs    [User: 5.9 µs, System: 10.3 µs]
Range (min … max):   494.1 µs … 1044.0 µs    5456 runs

Time (mean ± σ):     535.9 µs ±  51.6 µs    [User: 5.1 µs, System: 9.8 µs]
Range (min … max):   494.5 µs … 1071.3 µs    5285 runs
