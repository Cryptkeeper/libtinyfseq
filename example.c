#include <assert.h>
#include <stdio.h>

// define TINYFSEQ_IMPLEMENTATION to include inline function implementations
// otherwise only include "libtinyfseq.h"
// (in the style of stb headers: https://github.com/nothings/stb#how-do-i-use-these-libraries)
#define TINYFSEQ_IMPLEMENTATION
#include "tinyfseq.h"

static uint8_t FILE_DATA[] = {
        'P',
        'S',
        'E',
        'Q',
        0,// channelDataOffset
        0,
        0,// minorVersion
        2,// majorVersion
        0,// variableDataOffset
        0,
        0,// channelCount
        0,
        0,
        0,
        0,// frameCount
        0,
        0,
        0,
        0,  // frameStepTimeMillis
        0,  // -
        0,  // compressionType
        0,  // -
        0,  // channelRangeCount
        0,  // -
        'I',// sequenceUid
        'L',
        'O',
        'V',
        'E',
        'Y',
        'O',
        'U',
};

int main() {
    printf("using tinyfseq v%s\n", TINYFSEQ_VERSION);

    struct tf_file_header_t header;

    // read the "embedded" file, FILE_DATA
    enum tf_err_t err;
    if ((err = tf_read_file_header(FILE_DATA, sizeof(FILE_DATA), &header, NULL))) {
        return err;
    }

    // sequenceUid is an uint64_t that normally stores a timestamp
    // instead it carries an 8-byte string message
    // this assert call validates the decoded header prior to printing the char values
    assert(header.sequenceUid == 6147230170719669321);

    int bit_idx;
    for (bit_idx = 0; bit_idx < 64; bit_idx += 8) {
        uint64_t bit_mask = ((uint64_t) 0xFFu) << bit_idx;
        printf("%c", (uint8_t) ((header.sequenceUid & bit_mask) >> bit_idx));
    }

    printf("\n");

    return 0;
}
