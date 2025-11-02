#pragma once

#include <stddef.h>

typedef struct StrStream {
    size_t i;
    size_t text_size;
    char* text;
} StrStream;

StrStream str_stream_new(const char* str);

size_t str_stream_read(StrStream* this, char* buffer, size_t size);

void str_stream_free(StrStream* this);
