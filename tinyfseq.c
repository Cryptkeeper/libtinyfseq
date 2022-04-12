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
#include "tinyfseq.h"

#include <string.h>

const char *tf_err_str(enum tf_err_t err) {
#ifdef TF_INCLUDE_ERR_STRINGS
    switch (err) {
        case TF_OK:
            return "TF_OK (ok)";
        case TF_EINVALID_HEADER_SIZE:
            return "TF_EINVALID_HEADER_SIZE (undersized `tf_file_header_t` data decoding buffer)";
        case TF_EINVALID_MAGIC:
            return "TF_EINVALID_MAGIC (invalid magic file signature)";
        case TF_EINVALID_COMPRESSION_TYPE:
            return "TF_EINVALID_COMPRESSION_TYPE (compressed files are not supported)";
        case TF_EINVALID_VAR_HEADER_SIZE:
            return "TF_EINVALID_VAR_HEADER_SIZE (undersized `tf_var_header_t` data decoding buffer)";
        case TF_EINVALID_VAR_VALUE_SIZE:
            return "TF_EINVALID_VAR_VALUE_SIZE (undersized variable value data decoding buffer)";
        case TF_EINVALID_CHANNEL_RANGE_SIZE:
            return "TF_EINVALID_CHANNEL_RANGE_SIZE (undersized `tf_channel_range_t` data decoding buffer)";
        default:
            return "unknown `tf_err_t` value";
    }
#else
    return "NULL";
#endif
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

    // WARNING: this is vulnerable to breaking depending on how `tf_file_header_t` is packed/aligned
    // 15 bytes copies from the `.channelDataOffset` field to `.frameStepTimeMillis`
    // two additional calls each copy an individual field
    memcpy((unsigned char *) &header->channelDataOffset, &bd[4], 15);

    // upper 4 bits contain additional compression block count data that is ignored by tinyfseq
    // mask to lower 4 bits to filter only the compression type field
    if ((bd[20] & 0xF) != 0) {
        return TF_EINVALID_COMPRESSION_TYPE;
    }

    memcpy((unsigned char *) &header->channelRangeCount, &bd[22], 1);
    memcpy((unsigned char *) &header->sequenceUid, &bd[24], 8);

    if (ep != NULL) {
        *ep = ((uint8_t *) bd) + FILE_HEADER_SIZE;
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
        memcpy((unsigned char *) &varHeader->size, &bd[0], 4);

        // only attempt to read variable value if a decoding buffer (`vd`) is provided
        // `.size` already includes the 4 bytes the header consumes
        if (vd != NULL) {
            const int valueSize = varHeader->size - VAR_HEADER_SIZE;

            if (vs < valueSize) {
                return TF_EINVALID_VAR_VALUE_SIZE;
            } else {
                memcpy((unsigned char *) vd, &bd[VAR_HEADER_SIZE], valueSize);
            }
        }

        if (ep != NULL) {
            *ep = ((uint8_t *) bd) + varHeader->size;
        }

        return TF_OK;
    }
}

static uint32_t tf_read_uint24(const uint8_t *bd) {
    // WARNING: this assumes little endian byte order
    return (uint32_t) (bd[0] | (bd[1] << 8) | (bd[2] << 16));
}

enum tf_err_t tf_read_channel_range(const uint8_t *bd, int bs, struct tf_channel_range_t *channelRange, uint8_t **ep) {
    const int CHANNEL_RANGE_SIZE = 6;

    if (bs < CHANNEL_RANGE_SIZE) {
        return TF_EINVALID_CHANNEL_RANGE_SIZE;
    } else {
        channelRange->firstChannelNumber = tf_read_uint24(&bd[0]);
        channelRange->channelCount       = tf_read_uint24(&bd[3]);

        if (ep != NULL) {
            *ep = ((uint8_t *) bd) + CHANNEL_RANGE_SIZE;
        }

        return TF_OK;
    }
}

float tf_sequence_duration_seconds(uint32_t frameCount, uint8_t frameStepTimeMillis) {
    const unsigned int millis = frameCount * frameStepTimeMillis;
    return ((float) millis) / 1000.0F;
}
