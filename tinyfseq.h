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
/**
 * libtinyfseq
 * v2.1.0
 * https://github.com/Cryptkeeper/libtinyfseq
 *
 * Prior to including `tinyfseq.h`, two definition based options are available:
 *
 * 1. `TINYFSEQ_MEMCPY` allows you to override the selected `memcpy` function with whatever is best for your platform (currently a basic freestanding
 *    implementation, `tf_memcpy_impl`)
 * 2. `TINYFSEQ_STRIP_ERR_STRINGS` replaces all literal strings returned by `tf_err_str` with `"NULL"` (as a string) to
 *    reduce the compiled binary size
 *
 * `TINYFSEQ_IMPLEMENTATION` should be defined in a SINGLE C/C++ source code file to include the function implementations.
 * More information on using single-file libraries: https://github.com/nothings/stb#how-do-i-use-these-libraries
 */
#ifndef TINYFSEQ_H
#define TINYFSEQ_H

#include <stdint.h>

#define TINYFSEQ_VERSION "2.1.1"

enum tf_err_t {
    TF_OK = 0,
    TF_EINVALID_HEADER_SIZE,
    TF_EINVALID_MAGIC,
    TF_EINVALID_COMPRESSION_TYPE,
    TF_EINVALID_VAR_HEADER_SIZE,
    TF_EINVALID_VAR_VALUE_SIZE,
    TF_EINVALID_CHANNEL_RANGE_SIZE,
    TF_EINVALID_COMPRESSION_BLOCK_SIZE,
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
    uint8_t         compressionBlockCount;
    uint8_t         channelRangeCount;
    uint64_t        sequenceUid;
};

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

struct tf_compression_block_t {
    uint32_t firstFrameId;
    uint32_t size;
};

enum tf_err_t tf_read_compression_block(const uint8_t *bd, int bs, struct tf_compression_block_t *block, uint8_t **ep);

struct tf_var_header_t {
    uint16_t size;
    uint8_t  id[2];
};

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

#endif//TINYFSEQ_H

#ifdef TINYFSEQ_IMPLEMENTATION

static inline void tf_memcpy_impl(uint8_t *dst, const uint8_t *src, int count) {
    for (; count > 0; count--) {
        (*dst++) = (*src++);
    }
}

#define TINYFSEQ_MEMCPY tf_memcpy_impl

const char *tf_err_str(enum tf_err_t err) {
#ifndef TINYFSEQ_STRIP_ERR_STRINGS
    switch (err) {
        case TF_OK:
            return "TF_OK (ok)";
        case TF_EINVALID_HEADER_SIZE:
            return "TF_EINVALID_HEADER_SIZE (undersized `tf_file_header_t` data decoding buffer)";
        case TF_EINVALID_MAGIC:
            return "TF_EINVALID_MAGIC (invalid magic file signature)";
        case TF_EINVALID_COMPRESSION_TYPE:
            return "TF_EINVALID_COMPRESSION_TYPE (unknown compression format/unmapped `tf_ctype_t` value)";
        case TF_EINVALID_VAR_HEADER_SIZE:
            return "TF_EINVALID_VAR_HEADER_SIZE (undersized `tf_var_header_t` data decoding buffer)";
        case TF_EINVALID_VAR_VALUE_SIZE:
            return "TF_EINVALID_VAR_VALUE_SIZE (undersized variable value data decoding buffer)";
        case TF_EINVALID_CHANNEL_RANGE_SIZE:
            return "TF_EINVALID_CHANNEL_RANGE_SIZE (undersized `tf_channel_range_t` data decoding buffer)";
        case TF_EINVALID_COMPRESSION_BLOCK_SIZE:
            return "TF_EINVALID_COMPRESSION_BLOCK_SIZE (undersized `tf_compression_block_t` data decoding buffer)";
        default:
            return "unknown `tf_err_t` value";
    }
#else
    return "NULL";
#endif
}

static inline int tf_is_known_ctype(uint8_t b) {
    switch (b) {
        case TF_COMPRESSION_NONE:
        case TF_COMPRESSION_ZSTD:
        case TF_COMPRESSION_ZLIB:
            return 1;
        default:
            return 0;
    }
}

enum tf_err_t tf_read_file_header(const uint8_t *bd, int bs, struct tf_file_header_t *header, uint8_t **ep) {
    // header structure is a fixed 32 byte size according to schema
    // https://github.com/Cryptkeeper/fseq-file-format#header
    const int FILE_HEADER_SIZE = 32;

    if (bs < FILE_HEADER_SIZE) {
        return TF_EINVALID_HEADER_SIZE;
    }

    if (bd[0] != 'P' || bd[1] != 'S' || bd[2] != 'E' || bd[3] != 'Q') {
        return TF_EINVALID_MAGIC;
    }

    header->channelDataOffset   = ((uint16_t *) &bd[4])[0];
    header->minorVersion        = bd[6];
    header->majorVersion        = bd[7];
    header->variableDataOffset  = ((uint16_t *) &bd[8])[0];
    header->channelCount        = ((uint32_t *) &bd[10])[0];
    header->frameCount          = ((uint32_t *) &bd[14])[0];
    header->frameStepTimeMillis = bd[18];

    // upper 4 bits contain additional compression block count data that is ignored by tinyfseq
    // mask to lower 4 bits to filter only the compression type field
    const uint8_t compressionType = bd[20] & 0xF;

    if (!tf_is_known_ctype(compressionType)) {
        return TF_EINVALID_COMPRESSION_TYPE;
    }

    header->compressionType       = (enum tf_ctype_t) compressionType;
    header->compressionBlockCount = bd[21];

    header->channelRangeCount = bd[22];
    header->sequenceUid       = ((uint64_t *) &bd[24])[0];

    if (ep) {
        *ep = ((uint8_t *) bd) + FILE_HEADER_SIZE;
    }

    return TF_OK;
}

enum tf_err_t tf_read_compression_block(const uint8_t *bd, int bs, struct tf_compression_block_t *block, uint8_t **ep) {
    const int COMPRESSION_BLOCK_SIZE = 8;

    if (bs < COMPRESSION_BLOCK_SIZE) {
        return TF_EINVALID_COMPRESSION_BLOCK_SIZE;
    }

    block->firstFrameId = ((uint32_t *) &bd[0])[0];
    block->size         = ((uint32_t *) &bd[4])[0];

    if (ep) {
        *ep = ((uint8_t *) bd) + COMPRESSION_BLOCK_SIZE;
    }

    return TF_OK;
}

enum tf_err_t tf_read_var_header(const uint8_t *bd, int bs, struct tf_var_header_t *varHeader, uint8_t *vd, int vs, uint8_t **ep) {
    const int VAR_HEADER_SIZE = 4;

    // variable header requires 4 bytes and is NULL terminated
    // an empty variable should be at least 5 bytes
    if (bs <= VAR_HEADER_SIZE) {
        return TF_EINVALID_VAR_HEADER_SIZE;
    } else {
        varHeader->size = ((uint16_t *) &bd[0])[0];

        TINYFSEQ_MEMCPY(varHeader->id, &bd[2], sizeof(varHeader->id));

        // only attempt to read variable value if a decoding buffer (`vd`) is provided
        // `.size` already includes the 4 bytes the header consumes
        if (vd) {
            const int valueSize = varHeader->size - VAR_HEADER_SIZE;

            if (vs < valueSize) {
                return TF_EINVALID_VAR_VALUE_SIZE;
            } else {
                TINYFSEQ_MEMCPY(vd, &bd[VAR_HEADER_SIZE], valueSize);
            }
        }

        if (ep) {
            *ep = ((uint8_t *) bd) + varHeader->size;
        }

        return TF_OK;
    }
}

static inline uint32_t tf_read_uint24(const uint8_t *bd) {
    return (uint32_t) (bd[0] | (bd[1] << 8) | (bd[2] << 16));
}

enum tf_err_t tf_read_channel_range(const uint8_t *bd, int bs, struct tf_channel_range_t *channelRange, uint8_t **ep) {
    const int CHANNEL_RANGE_SIZE = 6;

    if (bs < CHANNEL_RANGE_SIZE) {
        return TF_EINVALID_CHANNEL_RANGE_SIZE;
    } else {
        channelRange->firstChannelNumber = tf_read_uint24(&bd[0]);
        channelRange->channelCount       = tf_read_uint24(&bd[3]);

        if (ep) {
            *ep = ((uint8_t *) bd) + CHANNEL_RANGE_SIZE;
        }

        return TF_OK;
    }
}

float tf_sequence_duration_seconds(uint32_t frameCount, uint8_t frameStepTimeMillis) {
    const unsigned int millis = frameCount * frameStepTimeMillis;

    return ((float) millis) / 1000.0F;
}

#endif//TINYFSEQ_IMPLEMENTATION
