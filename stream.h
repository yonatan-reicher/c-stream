#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "file-stream.h"
#include "str-stream.h"
#include "then-stream.h"
#include "char-stream.h"
#include "trim-stream.h"
#include "cmd-stream.h"

typedef enum StreamKind {
    SK_FREED = 0,
    SK_FILE,
    SK_STR,
    SK_THEN,
    SK_CHAR,
    SK_TRIM,
    SK_CMD,
} StreamKind;

/*
 * NOTE:
 * This struct is made of a union of a bunch of other structs. I am aiming for
 * each of those structs to be 3 words or below
 */
typedef struct Stream {
    StreamKind kind;
    union {
        FileStream file;
        StrStream str;
        ThenStream then;
        CharStream ch;
        TrimStream trim;
        CmdStream cmd;
    } inner;
} Stream;

// Stream operations

void stream_free(Stream* s);

size_t stream_read(Stream* s, char* buffer, size_t size);

void stream_read_all_to_file(Stream* s, FILE* out);

void stream_read_all_to_str(Stream* s, char** str, size_t* size);

bool stream_size(Stream* s, size_t* size);

// Stream constructors

Stream stream_file(const char* file_path);

Stream stream_str(const char* text);

Stream stream_then(Stream left, Stream right);

Stream stream_then_all(Stream* streams, size_t n);

Stream stream_char(char c);

/* Must have strlen < CHAR_STREAM_MAX_CHARS */
Stream stream_chars(char* c);

Stream stream_trim(Stream other);

void stream_cmd(
    const char* cmd, const char* const* args,
    Stream stdin_stream,
    Stream* stdout_stream, Stream* stderr_stream
);
