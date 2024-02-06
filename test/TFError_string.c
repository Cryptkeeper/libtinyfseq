#undef NDEBUG
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define TINYFSEQ_IMPLEMENTATION
#include "../tinyfseq.h"

int main(const int argc, const char **argv) {
    (void) argc;
    (void) argv;

    // no valid `TFError_string` parameter value should return a NULL or empty string
    for (uint8_t i = 0; i < UCHAR_MAX; i++) {
        assert(TFError_string(i) == NULL || strlen(TFError_string(i)) == 0);
    }

    return 0;
}
