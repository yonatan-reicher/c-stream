#include "char-stream.h"

#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CharStream char_stream_chars(char* cs) {
    size_t s = strlen(cs);
    if (s >= CHAR_STREAM_MAX_CHARS) {
        fprintf(stderr, "Bad char_stream_chars call! What?\n");
        exit(1);
    }
    CharStream this;
    memcpy(this.chars, cs, CHAR_STREAM_MAX_CHARS);
    return this;
}

CharStream char_stream_char(char c) {
    char a[CHAR_STREAM_MAX_CHARS] = { c, 0 };
    return char_stream_chars(a);
}

void char_stream_free(CharStream* this) {
    UNUSED(this);
}

size_t char_stream_read(CharStream* this, char* buffer, size_t size) {
    int n_read = 0;
    while (n_read < CHAR_STREAM_MAX_CHARS && this->chars[n_read] != 0) {
        if (size == 0) break;
        *buffer = this->chars[n_read];
        n_read++;
        size--;
        buffer++;
    }

    // Special case that's common and we can handle faster
    if (n_read == CHAR_STREAM_MAX_CHARS) {
        this->chars[0] = 0;
        return CHAR_STREAM_MAX_CHARS;
    }

    // Now, we shift the characters that we did not read back, and put zeros
    // after.
    int j;
    for (j = 0; j < n_read && j + n_read < CHAR_STREAM_MAX_CHARS; j++) {
        this->chars[j] = this->chars[j + n_read];
    }
    if (j < CHAR_STREAM_MAX_CHARS) this->chars[j] = 0;

    return n_read;
}
