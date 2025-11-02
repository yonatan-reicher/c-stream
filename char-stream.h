#pragma once

#include <stddef.h>
#include <stdint.h>

#define CHAR_STREAM_MAX_CHARS (8 * 3)

/* This actually supports multiple chars. */
typedef struct CharStream {
    char chars[CHAR_STREAM_MAX_CHARS];
} CharStream;

CharStream char_stream_char(char c);

/* Must have strlen < CHAR_STREAM_MAX_CHARS */
CharStream char_stream_chars(char* cs);

void char_stream_free(CharStream* this);

size_t char_stream_read(CharStream* this, char* buffer, size_t size);
