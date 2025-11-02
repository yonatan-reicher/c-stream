#pragma once

#include "common.h"
#include <stddef.h>
#include <stdio.h>

typedef struct FileStream {
    FILE* file;
    size_t file_path_size;
    char* file_path;
} FileStream;

FileStream file_stream_new(const char* file_path);

size_t file_stream_read(FileStream* this, char* buffer, size_t size);

void file_stream_free(FileStream* this);

bool file_stream_size(FileStream* this, size_t* size);
