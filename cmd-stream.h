#pragma once

#include "common.h"

typedef struct Stream Stream;

typedef struct CmdStreamData CmdStreamData;

typedef enum CmdStreamKind : uint8_t {
    CS_KIND_STDOUT,
    CS_KIND_STDERR,
} CmdStreamKind;

/* This actually will double as two streams: both the output and the error. */
typedef struct CmdStream {
    CmdStreamData* data;
    int fd;
    CmdStreamKind kind;
} CmdStream;

void cmd_stream_new(
    const char* cmd,
    size_t cmd_size,
    const char* const* args,
    const size_t* arg_sizes,
    size_t n_args,
    Stream stdin_stream,
    CmdStream* stdout_stream,
    CmdStream* stderr_stream
);

void cmd_stream_free(CmdStream* cmd_stream);

size_t cmd_stream_read(CmdStream* this, char* buffer, size_t size);
