/*
* MIT License
*
* Copyright (c) 2022 Nick Krecklow
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef LIBTINYFSEQ_TINYFSEQ_HIN
#define LIBTINYFSEQ_TINYFSEQ_HIN

#include <stdint.h>

#define TF_LIB_VERSION "1.2.0"

enum tf_err_t {
    TF_OK = 0,
    TF_EINVALID_HEADER_SIZE,
    TF_EINVALID_MAGIC,
    TF_EINVALID_COMPRESSION_TYPE,
    TF_EINVALID_VAR_HEADER_SIZE,
    TF_EINVALID_VAR_VALUE_SIZE,
    TF_EINVALID_CHANNEL_RANGE_SIZE,
};

/**
 * `tf_err_str` returns a string version of the enum name, including a comment with additional error context.
 * `tf_err_str` does not allocate, its return values should not be freed. `tf_err_str` will never return a NULL value.
 *
 * @param err Unknown/invalid values will default to returning "unknown `tf_err_t` value"
 * @return A string version of the given value, or "NULL" (as a string) if the library is built with `TF_STRIP_ERR_STRINGS` defined
 */
const char *tf_err_str(enum tf_err_t err);

enum tf_ctype_t {
    TF_COMPRESSION_NONE,
    TF_COMPRESSION_ZSTD,
    TF_COMPRESSION_ZLIB,
};

struct tf_file_header_t {
    uint16_t        channelDataOffset;
    uint8_t         minorVersion;
    uint8_t         majorVersion;
    uint16_t        variableDataOffset;
    uint32_t        channelCount;
    uint32_t        frameCount;
    uint8_t         frameStepTimeMillis;
    enum tf_ctype_t compressionType;
    uint8_t         channelRangeCount;
    uint64_t        sequenceUid;
} __attribute__((packed));

/**
 * `tf_read_file_header` selectively decodes the fields available within `struct tf_file_header_t` from a given data buffer.
 * The buffer should be at least 32 bytes in length (the minimum size of a FSEQ v2.0+ header).
 *
 * @param bd Data buffer to decode into the given `struct tf_file_header_t` pointer
 * @param bs Size of the data buffer (>=32)
 * @param header `struct tf_file_header_t` instance to decode data buffer fields into
 * @param ep End pointer which upon a successful return will point to the end of the structure within `bd` (may be NULL)
 * @return A `tf_err_t` value indicating an error, if any, otherwise `TF_OK`
 */
enum tf_err_t tf_read_file_header(const uint8_t *bd, int bs, struct tf_file_header_t *header, uint8_t **ep);

struct tf_var_header_t {
    uint16_t size;
    uint8_t  id[2];
} __attribute__((packed));

/**
 * `tf_read_var_header` decodes the fields available within `struct tf_var_header_t` from a given data buffer.
 * The buffer should be at least 5 bytes in length (the minimum size of an encoded variable header).
 *
 * An additional data buffer (`vd`) is used to store the decoded variable value (compared to `struct tf_var_header_t` which
 * only contains the header fields). A NULL `vd` value will skip this decoding step.
 *
 * @param bd Data buffer to decode into the given `struct tf_var_header_t` pointer
 * @param bs Size of the data buffer (>=6)
 * @param varHeader `struct tf_var_header_t` instance to decode data buffer fields into
 * @param vd Optional data buffer for storing the corresponding value of the variable header (may be NULL)
 * @param vs Size of the optional data buffer
 * @param ep End pointer which upon a successful return will point to the end of the structure within `bd` (may be NULL)
 * @return A `tf_err_t` value indicating an error, if any, otherwise `TF_OK`
 */
enum tf_err_t tf_read_var_header(const uint8_t *bd, int bs, struct tf_var_header_t *varHeader, uint8_t *vd, int vs, uint8_t **ep);

struct tf_channel_range_t {
    uint32_t firstChannelNumber;
    uint32_t channelCount;
};

/**
 * `tf_read_channel_range` decodes the fields available within `struct tf_channel_range_t` from a given data buffer.
 * The buffer should be at least 6 bytes in length (the fixed size of an encoded channel range).
 *
 * @param bd Data buffer to decode into the given `struct tf_channel_range_t` pointer
 * @param bs Size of the data buffer (>=6)
 * @param channelRange `struct tf_channel_range_t` instance to decode data buffer fields into
 * @param ep End pointer which upon a successful return will point to the end of the structure within `bd` (may be NULL)
 * @return A `tf_err_t` value indicating an error, if any, otherwise `TF_OK`
 */
enum tf_err_t tf_read_channel_range(const uint8_t *bd, int bs, struct tf_channel_range_t *channelRange, uint8_t **ep);

/**
 * Calculates the seconds duration of a sequence in seconds given its frame step time and frame count.
 * `frameCount` and `frameStepTimeMillis` are available via a `struct tf_file_header_t` value.
 *
 * @param frameCount The number of frames within a sequence
 * @param frameStepTimeMillis The duration (in milliseconds) of each frame
 * @return The duration of the sequence in seconds
 */
float tf_sequence_duration_seconds(uint32_t frameCount, uint8_t frameStepTimeMillis);

#endif//LIBTINYFSEQ_TINYFSEQ_HIN
