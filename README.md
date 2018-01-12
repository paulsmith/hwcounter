# hwcounter

Highly accurate counter for measuring elapsed time in Python.

## Installation

``` shell
$ pip install hwcounter
```

## Overview

This Python extension module uses the hardware timestamp counter to
provide very high precision and accurate measurements of execution
time.

The module exposes three main objects. Two are plain functions,
`count` and `count_end`, which return the current value of the
timestamp counter. They can be used together to manually track the
start and end of a timing measurement. The third is the `Timer` type,
which is to be used as a context manager, wrapping a block of code to
be measured.

Using `Timer` to wrap a block of code is to be preferred over
manually using `count` and `count_end`. This is for convenience, as well
as because it factors in the overhead of calling the underlying
measurement instructions.

### `count()`

Returns the current value of the timestamp counter, in cycles.

### `count_end()`

Returns the current value of the timestamp counter, in cycles. It is
suitable for use at the end of a timing measurement.

### `Timer`

Class that implements the context manager protocol.

#### `Timer.cycles`

The `cycles` attribute is populated with the elapsed time, in cycles,
when the Timer context manager exits.

## Example usage

``` python
from hwcounter import Timer, count, count_end

from time import sleep
from math import sqrt


# 1. Manually count cycles elapsed between two points

start = count()
sqrt(144) / 12
elapsed = count_end() - start
print(f'elapsed cycles: {elapsed}')
# elapsed cycles: 36486


# 2. Use Timer object as context manager to wrap a block of code and measure its timing

with Timer() as t:
	sleep(1)
print(f'elapsed cycles: {t.cycles}')
# elapsed cycles: 2912338344
```

These examples were performed on an Intel Core i5-6267U CPU @
2.90GHz. Notice that the sleep for 1 second in the example above
yields a 2.9 billion cycle count.

## Notes

The overhead of calling the underlying measurement instructions is
taken into account when using the Timer context manager. In other
words, the number of cycles it takes to call the machine instructions
are subtracted from the elapsed cycle count automatically.

This library returns measurements in processor clock cycles. For
benchmarking programs and making apples-to-apples comparisons of
changes in code execution time, this method is sufficient and
reliable. If elapsed time in seconds is desired, a conversion from
clock cycles is required: divide the cycle count by the processor's
clock speed (in Hz). This conversion is outside the scope of this
module.

## Portability

This extension uses the `RDTSC` and `RDTSCP` instructions on the x86
architecture, so it won't work on other platforms. It is Python 3
only.
