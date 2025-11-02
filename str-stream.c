#include "str-stream.h"
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

StrStream str_stream_new(const char* str) {
    StrStream this;
    this.i = 0;
    this.text_size = strlen(str);
    this.text = strdup(str);
    return this;
}

void str_stream_free(StrStream* this) {
    free(this->text);
}

size_t str_stream_read(StrStream* this, char* buffer, size_t size) {
    size_t n_read = MIN(size, this->text_size - this->i);
    memcpy(buffer, this->text + this->i, n_read);
    this->i += n_read;
    return n_read;
}
