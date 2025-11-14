#include "then-stream.h"

#include "stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ThenStream then_stream_from_array(Stream* children, size_t n) {
    ThenStream this;
    this.n = n;
    this.i = 0;
    this.children = (Stream*)malloc(n * sizeof(Stream));
    memcpy(this.children, children, n * sizeof(Stream));
    memset(children, 0, n * sizeof(Stream));
    return this;
}

void then_stream_free(ThenStream* this) {
    for (size_t i = 0; i < this->n; i++) {
        stream_free(&this->children[i]);
    }
    free(this->children);
}

ThenStream then_stream_from_two(Stream left, Stream right) {
    Stream a[2] = { left, right };
    return then_stream_from_array(a, 2);
}

size_t then_stream_read(ThenStream* this, char* buffer, size_t size) {
    // First, we try and read. If the read succeeds, we are done.
    // If it failed, and there is another stream, advance and repeat.
    // Else, it failed and there is no other stream. We are done!
    while (true) {
        if (this->i >= this->n) return 0;
        size_t n_read = stream_read(&this->children[this->i], buffer, size);
        if (n_read > 0) return n_read;
        this->i++;
    }
}

bool then_stream_size(ThenStream* this, size_t* size) {
    size_t total;
    size_t current;
    for (size_t i = 0; i < this->n; i++) {
        bool has_size = stream_size(&this->children[i], &current);
        if (!has_size) return false;
        total += current;
    }
    *size = total;
    return true;
}
