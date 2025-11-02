#pragma once

#include "common.h"

typedef struct Stream Stream;

typedef struct TrimStream {
    Stream* other;
    char* saved_for_later;
    uint32_t saved_for_later_size;
    uint16_t saved_for_later_offset;
    enum : uint16_t {
        TS_STATE_JUST_BEGAN = 0,
        TS_STATE_NORMAL,
        TS_STATE_ALREADY_ENDED,
    } state;
} TrimStream;

TrimStream trim_stream_new(Stream other);

void trim_stream_free(TrimStream* this);

size_t trim_stream_read(TrimStream* this, char* buffer, size_t size);
