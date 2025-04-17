# parser benchmarks

## parser tests

### Before

Time (mean ± σ):       7.3 ms ±   0.4 ms    [User: 2.9 ms, System: 2.5 ms]
Range (min … max):     6.8 ms …   9.5 ms    318 runs

### STRCMP macro

Time (mean ± σ):       7.2 ms ±   0.4 ms    [User: 2.8 ms, System: 2.5 ms]
Range (min … max):     6.7 ms …   9.2 ms    326 runs

### Avoid memcmp with macros

Time (mean ± σ):       7.2 ms ±   0.3 ms    [User: 2.8 ms, System: 2.5 ms]
Range (min … max):     6.7 ms …   9.4 ms    418 runs

### Globals

Time (mean ± σ):       7.2 ms ±   0.4 ms    [User: 2.6 ms, System: 2.6 ms]
Range (min … max):     6.6 ms …   9.3 ms    404 runs

Time (mean ± σ):       7.2 ms ±   0.3 ms    [User: 2.8 ms, System: 2.5 ms]
Range (min … max):     6.8 ms …   9.5 ms    408 runs

## parser bench

### bench: Before

Time (mean ± σ):       4.4 ms ±   0.2 ms    [User: 2.3 ms, System: 0.6 ms]
Range (min … max):     4.0 ms …   5.8 ms    633 runs

### bench: Avoid memcmp with macros

Time (mean ± σ):       4.3 ms ±   0.3 ms    [User: 2.3 ms, System: 0.6 ms]
Range (min … max):     3.9 ms …   6.3 ms    724 runs

### bench: After parser_state global

Time (mean ± σ):       4.1 ms ±   0.3 ms    [User: 2.1 ms, System: 0.6 ms]
Range (min … max):     3.6 ms …   6.2 ms    634 runs

Time (mean ± σ):       4.1 ms ±   0.3 ms    [User: 2.0 ms, System: 0.6 ms]
Range (min … max):     3.6 ms …   6.5 ms    745 runs

### bench: after buf_pos and assignment_pos global

Time (mean ± σ):       4.0 ms ±   0.3 ms    [User: 2.0 ms, System: 0.5 ms]
Range (min … max):     3.5 ms …   5.3 ms    684 runs

### bench: after buffer and var_name global

Time (mean ± σ):       3.9 ms ±   0.2 ms    [User: 1.9 ms, System: 0.5 ms]
Range (min … max):     3.6 ms …   6.7 ms    573 runs

