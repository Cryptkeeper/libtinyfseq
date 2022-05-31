# libtinyfseq

A tiny library (~125 LOC) for decoding FSEQ (.fseq) v2.0+ sequence files developed and popularized by
the [fpp](https://github.com/FalconChristmas/fpp) and [xLights](https://github.com/smeighan/xLights) programs.
Additional documentation for the file format is available
at [Cryptkeeper/fseq-file-format](https://github.com/Cryptkeeper/fseq-file-format).

## Installation

libtinyfseq uses [CMake](https://cmake.org/) for building and packaging the library.

- Generate Makefiles using `cmake .`
- Compile the library using `make`
- Optionally install the headers and compiled archive using `make install`
- Include in your project using `#include <tinyfseq/tinyfseq.h>`

If optionally installed, `install_manifest.txt` will be created, containing the installed file paths for easy removal.

## Build Configuration

For devices with limited memory, a `TF_INCLUDE_ERR_STRINGS` definition is included in [CMakeLists.txt](CMakeLists.txt)
to disable the inclusion of error strings in the build. Calls to `tf_err_str` will instead return `"NULL"` (as a string).

## Compatibility

- libtinyfseq only supports FSEQ versions v2.x versions, with the schema initially released in 2018. Older v1.x files
  can be upgraded using the [xLights](https://github.com/smeighan/xLights) program.
- To minimize dependencies, libtinyfseq does not support compressed FSEQ files as they may use both
  [zstd](https://github.com/facebook/zstd) and [zlib](https://www.zlib.net)
  compression. Compressed FSEQ files can be pre-decompressed using the [xLights](https://github.com/smeighan/xLights)
  program, or you may decompress the data buffer yourself using an additional library before passing it to
  libtinyfseq.
- libtinyfseq assumes data buffers are in little endian byte order.

## Usage

libtinyfseq only defines three functions for reading the various components of a FSEQ file. See [tinyfseq.h](tinyfseq.h)
for comments describing their usage.

| Function | Schema |
| --- | --- |
| `tf_read_file_header` | https://github.com/Cryptkeeper/fseq-file-format#header |
| `tf_read_var_header` | https://github.com/Cryptkeeper/fseq-file-format#variable |
| `tf_read_channel_range` | https://github.com/Cryptkeeper/fseq-file-format#sparse-range |

Two additional utility functions are provided:

1. `tf_sequence_duration_seconds` for calculating the duration of a given sequence in seconds
2. `tf_err_str` for mapping `enum tf_err_t` values into their string names

## License

See [LICENSE.txt](LICENSE.txt)
