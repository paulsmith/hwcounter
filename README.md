# hwcounter

Highly accurate timer for measuring code execution in Python

## Example usage

``` python
from hwcounter import Timer, count

# Manually count cycles elapsed between two points

start = count()
# ... code execution to be measured
elapsed = count() - start
printf(f'elapsed cycles: {elapsed}')


# Use Timer object as context manager to wrap a block of code and measure its timing

with Timer() as t:
    # ... code block to time
printf(f'elapsed cycles: {t.cycles}')
```

## Portability

This extension uses the `RDTSC` and `RDTSCP` instructions on the x86 architecture, so it won't work on other platforms. It is Python 3 only.
