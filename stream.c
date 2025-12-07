#include "stream.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stream_free(Stream* this) {
    switch (this->kind) {
        case SK_FREED: return;
        case SK_FILE: file_stream_free(&this->inner.file); break;
        case SK_STR: str_stream_free(&this->inner.str); break;
        case SK_THEN: then_stream_free(&this->inner.then); break;
        case SK_CHAR: char_stream_free(&this->inner.ch); break;
        case SK_TRIM: trim_stream_free(&this->inner.trim); break;
        case SK_CMD: cmd_stream_free(&this->inner.cmd); break;
    }
    memset(this, 0, sizeof(Stream));
}

size_t stream_read(Stream* this, char* buffer, size_t size) {
    switch (this->kind) {
        case SK_FREED: return 0;
        case SK_FILE: return file_stream_read(&this->inner.file, buffer, size);
        case SK_STR: return str_stream_read(&this->inner.str, buffer, size);
        case SK_THEN: return then_stream_read(&this->inner.then, buffer, size);
        case SK_CHAR: return char_stream_read(&this->inner.ch, buffer, size);
        case SK_TRIM: return trim_stream_read(&this->inner.trim, buffer, size);
        case SK_CMD: return cmd_stream_read(&this->inner.cmd, buffer, size);
    }
    return 0;
}

void stream_read_all_to_file(Stream s, FILE* out) {
    // Special case for string streams!
    if (s.kind == SK_STR) {
        fwrite(s.inner.str.text, 1, s.inner.str.text_size, out);
        goto end;
    }
#define BUF_SIZE 1024
    char buffer[BUF_SIZE];
    while (true) {
        size_t n_read = stream_read(&s, buffer, BUF_SIZE);
        fwrite(buffer, 1, n_read, out);
        if (n_read == 0) {
            goto end;
        }
    }
#undef BUF_SIZE
end:
    stream_free(&s);
}

void stream_read_all_to_str(Stream this, char** str, size_t* size) {
    // Special case for string streams!
    if (this.kind == SK_STR) {
        *str = this.inner.str.text;
        *size = this.inner.str.text_size;
        // Return without calling `stream_free` because the only owned resource
        // is `inner.str.text`, which we return and do not want to free!
        return;
    }
    // Special case for sized streams!
    size_t my_size = 0;
    if (stream_size(&this, &my_size)) {
        *str = (char*)malloc(my_size);
        *size = my_size;
        // Now move the memory
        size_t n_read_total = 0;
        while (n_read_total < my_size) {
            size_t n_read = stream_read(
                &this,
                *str + n_read_total,
                my_size - n_read_total
            );
            n_read_total += n_read;
        }
        goto end;
    }
#define CHUNK_SIZE 1024
    char* buffer = malloc(CHUNK_SIZE);
    size_t buffer_size = CHUNK_SIZE;
    size_t n_read_total = 0;
    while (true) {
        size_t n_read = stream_read(
            &this,
            buffer + n_read_total,
            buffer_size - n_read_total
        );
        if (n_read == 0) {
            *str = buffer;
            *size = n_read_total;
            return;
        }
        n_read_total += n_read;
        if (n_read_total == buffer_size) {
            buffer_size += CHUNK_SIZE;
            buffer = realloc(buffer, buffer_size);
        }
    }
#undef CHUNK_SIZE
end:
    stream_free(&this);
}

Stream stream_file(const char* file_path) {
    Stream this;
    this.kind = SK_FILE;
    this.inner.file = file_stream_new(file_path);
    return this;
}

Stream stream_str(const char* str) {
    Stream this;
    this.kind = SK_STR;
    this.inner.str = str_stream_new(str);
    return this;
}

Stream stream_then(Stream left, Stream right) {
    Stream this;
    this.kind = SK_THEN;
    this.inner.then = then_stream_from_two(left, right);
    return this;
}

Stream stream_then_all(Stream* streams, size_t n) {
    Stream this;
    this.kind = SK_THEN;
    this.inner.then = then_stream_from_array(streams, n);
    return this;
}

Stream stream_char(char c) {
    Stream this;
    this.kind = SK_CHAR;
    this.inner.ch = char_stream_char(c);
    return this;
}

Stream stream_chars(char* c) {
    if (strlen(c) >= CHAR_STREAM_MAX_CHARS) {
        fprintf(stderr, "Bad stream_chars call!");
        exit(1);
    }
    Stream this;
    this.kind = SK_CHAR;
    this.inner.ch = char_stream_chars(c);
    return this;
}

bool stream_size(Stream* this, size_t* size) {
    switch (this->kind) {
        case SK_FREED: return false;
        case SK_FILE: return file_stream_size(&this->inner.file, size);
        case SK_STR:
            *size = this->inner.str.text_size;
            return true;
        case SK_THEN: return then_stream_size(&this->inner.then, size);
        case SK_CHAR: return false;
        case SK_TRIM: return false;
        case SK_CMD: return false;
    }
    return false;
}

Stream stream_trim(Stream other) {
    Stream this;
    this.kind = SK_TRIM;
    this.inner.trim = trim_stream_new(other);
    return this;
}

void stream_cmd(
    const char* cmd, const char* const* args,
    Stream stdin_stream,
    Stream* stdout_stream, Stream* stderr_stream
) {
    size_t n_args = 0;
    while (args[n_args] != NULL) n_args++;
    size_t* sizes = alloca(n_args * sizeof(size_t));
    for (size_t i = 0; i < n_args; i++) {
        sizes[i] = strlen(args[i]);
    }
    cmd_stream_new(
        cmd,
        strlen(cmd),
        args,
        sizes,
        n_args,
        stdin_stream,
        &stdout_stream->inner.cmd,
        &stderr_stream->inner.cmd
    );
    stdout_stream->kind = SK_CMD;
    stderr_stream->kind = SK_CMD;
}
