# libtinyfseq

A single-file library (~150 LOC) for decoding FSEQ (.fseq) v2.0+ sequence files developed and popularized by
the [fpp](https://github.com/FalconChristmas/fpp) and [xLights](https://github.com/smeighan/xLights) programs.
Additional documentation for the file format is available
at [Cryptkeeper/fseq-file-format](https://github.com/Cryptkeeper/fseq-file-format).

## Installation

- Download and copy `tinyfseq.h` into your project locally, or to your toolchain's include paths
- `#include "tinyfseq.h"` as expected (you may need to modify the path).
- Define `TINYFSEQ_IMPLEMENTATION` in a single C/C++ source code
  file ([more info on using single-file libraries](https://github.com/nothings/stb#how-do-i-use-these-libraries))

A short example of including libtinyfseq and decoding a file header is available in [`example.c`](example.c)

## Library Configuration

Prior to including `tinyfseq.h`, two definition based options are available:

1. `TINYFSEQ_MEMCPY` allows you to override the selected `memcpy` function with whatever is best for your platform (
   currently a basic freestanding implementation, `tf_memcpy_impl`)
2. `TINYFSEQ_STRIP_ERR_STRINGS` replaces all literal strings returned by `tf_err_str` with `"NULL"` (as a string) to
   reduce the compiled binary size

## Compatibility

- libtinyfseq uses `stdint.h` for fixed-size int types
- libtinyfseq only supports FSEQ versions v2.x versions, with the schema initially released in 2018. Older v1.x files
  can be upgraded using the [xLights](https://github.com/smeighan/xLights) program.

## Usage

libtinyfseq only defines three functions for reading the various components of a FSEQ file. See [tinyfseq.h](tinyfseq.h)
for comments describing their usage.

| Function                | Schema                                                       |
| ----------------------- | ------------------------------------------------------------ |
| `tf_read_file_header`   | https://github.com/Cryptkeeper/fseq-file-format#header       |
| `tf_read_var_header`    | https://github.com/Cryptkeeper/fseq-file-format#variable     |
| `tf_read_channel_range` | https://github.com/Cryptkeeper/fseq-file-format#sparse-range |

Two additional utility functions are provided:

1. `tf_sequence_duration_seconds` for calculating the duration of a given sequence in seconds
2. `tf_err_str` for mapping `enum tf_err_t` values into their string names

## License

See [LICENSE.txt](LICENSE.txt)
