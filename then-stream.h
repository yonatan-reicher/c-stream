#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Stream Stream;

typedef struct ThenStream {
    uint32_t n;
    uint32_t i;
    Stream* children;
} ThenStream;

ThenStream then_stream_from_two(Stream left, Stream right);

ThenStream then_stream_from_array(Stream* children, size_t n);

size_t then_stream_read(ThenStream* this, char* buffer, size_t size);

void then_stream_free(ThenStream* this);

bool then_stream_size(ThenStream* this, size_t* size);
