#include "trim-stream.h"

#include "stream.h"
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

TrimStream trim_stream_new(Stream other) {
    TrimStream this;
    this.other = malloc(sizeof(Stream));
    *this.other = other;
    this.saved_for_later = NULL;
    this.saved_for_later_size = 0;
    this.saved_for_later_offset = 0;
    this.state = TS_STATE_JUST_BEGAN; 
    return this;
}

void trim_stream_free(TrimStream* this) {
    stream_free(this->other);
    if (this->saved_for_later) free(this->saved_for_later);
    free(this->other);
}

bool is_whitespace(char c) {
    switch (c) {
        case ' ':
        case '\f':
        case '\n':
        case '\t':
        case '\r':
        case '\v':
            return true;
        default:
            return false;
    }
}

// TODO: I was working on having offset for the saved_for_later buffer and using
// it for reading instead of directly from the stream.

size_t trim_stream_left_trim(TrimStream* this, char* buffer, size_t size) {
    // At this point, we have just start started reading from the stream. No
    // saved_for_later buffer yet, that's for sure.
again:
    size_t n_read = stream_read(this->other, buffer, size);
    if (n_read == 0) {
        this->state = TS_STATE_ALREADY_ENDED;
        return 0;
    }
    // Left trim.
    size_t i = 0;
    while (i < n_read && is_whitespace(buffer[i])) i++;
    if (i == n_read) goto again;
    // We have reached a non-whitespace character. Shift the buffer back.
    for (size_t j = 0; j < n_read - i; j++) buffer[j] = buffer[j + i];
    this->state = TS_STATE_NORMAL;
    return n_read - i;
}

size_t trim_stream_read_saved(TrimStream* this, char* buffer, size_t size) {
    const size_t saved_for_later_size = this->saved_for_later_size;
    const size_t n_read = MIN(size, saved_for_later_size);
    memcpy(buffer, this->saved_for_later + this->saved_for_later_offset, n_read);
    if (n_read < saved_for_later_size) {
        this->saved_for_later_size -= n_read;
        this->saved_for_later_offset += n_read;
    } else {
        free(this->saved_for_later);
        this->saved_for_later_offset = 0;
        this->saved_for_later_size = 0;
        this->saved_for_later = NULL;
    }
    return n_read;
}

bool we_have_non_whitespace_saved(const TrimStream* this) {
    char* read = this->saved_for_later + this->saved_for_later_offset;
    size_t until = this->saved_for_later_size;
    for (size_t i = 0; i < until; i++) {
        if (!is_whitespace(read[i])) {
            return true;
        }
    }
    return false;
}

bool read_more_until_eof_or_non_whitespace(TrimStream* this) {
again:
#define BUF_SIZE 1024
    char buffer[BUF_SIZE];
    size_t n_read = stream_read(this->other, buffer, BUF_SIZE);
    if (n_read == 0) {
        this->state = TS_STATE_ALREADY_ENDED;
        return false;
    }
    this->saved_for_later = realloc(
        this->saved_for_later,
        this->saved_for_later_size + this->saved_for_later_offset + n_read
    );
    memcpy(
        this->saved_for_later + this->saved_for_later_offset,
        buffer,
        n_read
    );
    this->saved_for_later_size += n_read;
    for (size_t i = 0; i < n_read; i++) {
        if (is_whitespace(buffer[i])) {
            return true;
        }
    }
    goto again;
#undef BUF_SIZE
}

size_t trim_stream_trim_right(char* buffer, size_t size) {
    // Find index of last non-whitespace character
    ssize_t i = size - 1;
    while (0 <= i && is_whitespace(buffer[i])) i--;
    return i + 1;
}

size_t trim_stream_read(TrimStream* this, char* buffer, size_t size) {
    size_t n_read =
        this->state == TS_STATE_JUST_BEGAN ?
            trim_stream_left_trim(this, buffer, size) :
        this->saved_for_later ?
            trim_stream_read_saved(this, buffer, size) :
        stream_read(this->other, buffer, size);
    if (is_whitespace(buffer[n_read - 1])) {
        if (we_have_non_whitespace_saved(this)) {
            return n_read;
        }
        bool has_non_whitespace = read_more_until_eof_or_non_whitespace(this);
        if (has_non_whitespace) {
            return n_read;
        } else {
            return trim_stream_trim_right(buffer, n_read);
        }
    }
    return n_read;
}
