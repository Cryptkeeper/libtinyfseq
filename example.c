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

int main(const int argc, char **const argv) {
    (void) argc;
    (void) argv;

    printf("using tinyfseq v%s\n", TINYFSEQ_VERSION);

    TFHeader header;
    TFError err;

    // read the "embedded" file, FILE_DATA
    if ((err = TFHeader_read(FILE_DATA, sizeof(FILE_DATA), &header, NULL))) {
        printf("libtinyfseq error: %s\n", TFError_string(err));

        return 1;
    }

    // TODO: use header fields, read variables, etc.

    // sequenceUid is an uint64_t that normally stores a timestamp
    // instead it carries an 8-byte string message
    // this assert call validates the decoded header prior to printing the char values
    assert(header.sequenceUid == 6147230170719669321);

    for (int i = 0; i < 64; i += 8) {
        const uint64_t mask = (uint64_t) 0xFFu << i;

        printf("%c", (uint8_t) ((header.sequenceUid & mask) >> i));
    }

    printf("\n");

    return 0;
}
