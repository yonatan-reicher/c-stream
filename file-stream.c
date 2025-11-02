#include "file-stream.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FileStream file_stream_new(const char* file_path) {
    FileStream this;
    this.file = fopen(file_path, "r");
    this.file_path_size = strlen(file_path);
    this.file_path = strdup(file_path);
    return this;
}

size_t file_stream_read(FileStream* this, char* buffer, size_t size) {
    size_t n_read = fread(buffer, 1, size, this->file);
    return n_read;
}

void file_stream_free(FileStream* this) {
    fclose(this->file);
    free(this->file_path);
}

bool file_stream_size(FileStream* this, size_t* size) {
    int errno_before = errno;
    // Save the position, seek to the end, check the new position, and come
    // back. If something fails, restore and bye bye.
    off_t pos = ftello(this->file);
    if (pos == -1) goto bad_exit;
    int seek_ret = fseeko(this->file, 0, SEEK_END);
    if (seek_ret == -1) goto bad_exit;
    off_t file_size = ftello(this->file);
    if (file_size == -1) {
        fseeko(this->file, pos, SEEK_SET);
        goto bad_exit;
    }
    *size = file_size;
    return true;
bad_exit:
    errno = errno_before;
    return false;
}
