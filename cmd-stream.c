#include "cmd-stream.h"

#include "stream.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct CmdStreamData {
    const char* cmd;
    const char* const* args;
    size_t n_args;
    pid_t feeder_child;
    pid_t cmd_child;
    uint8_t ref_count;
} CmdStreamData;

/* Opens a child process that takes input from the stream and puts it into the
 * returned pipe */
void open_feeder_child(Stream stdin_stream, pid_t* child_pid, int* out_fd) {
    // pipe
    int fds[2];
    pipe(fds);
    int read_fd = fds[0];
    int write_fd = fds[1];
    // fork
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        close(read_fd);
#define BUF_SIZE 1024
        char buffer[BUF_SIZE];
        while (true) {
            size_t n_read = stream_read(&stdin_stream, buffer, BUF_SIZE);
            if (n_read == 0) break;
            // TODO: Handle pipe closed or done
            write(write_fd, buffer, n_read);
        }
        close(write_fd);
        stream_free(&stdin_stream);
        exit(0);
#undef BUF_SIZE
    } else {
        // Parent
        close(write_fd);
        // TODO: This is probably wrong. We are basically using two copies of
        // the input stream, this can't be good. What if it's reading from a
        // pipe or a socket?
        stream_free(&stdin_stream);
        *child_pid = pid;
        *out_fd = read_fd;
        return;
    }
}

/* Opens the process that runs the command */
void open_cmd_child(
    int read_fd,
    const char* cmd,
    const char* const * args,
    pid_t* child_pid,
    int* stdout_read,
    int* stderr_read
) {
    int stdout_fds[2];
    int stderr_fds[2];
    // TODO: Handle failure
    pipe(stdout_fds);
    pipe(stderr_fds);
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        close(stdout_fds[0]);
        close(stderr_fds[0]);
        dup2(read_fd, STDIN_FILENO);
        dup2(stdout_fds[1], STDOUT_FILENO);
        dup2(stderr_fds[1], STDERR_FILENO);
        execvp(cmd, (char* const*)args);
        exit(69);
    } else {
        // Parent
        close(stdout_fds[1]);
        close(stderr_fds[1]);
        *stdout_read = stdout_fds[0];
        *stderr_read = stderr_fds[0];
        *child_pid = pid;
    }
}

static void* allocate_enough_memory(
    size_t cmd_size,
    const size_t* arg_sizes,
    size_t n_args
) {
    size_t size_for_strings = 0;
    size_for_strings += cmd_size + 1;
    for (size_t i = 0; i < n_args; i++) size_for_strings += arg_sizes[i] + 1;
    void* mem = malloc(
        sizeof(CmdStreamData)
        + (n_args + 1) * sizeof(char*)
        + size_for_strings
    );
    return mem;
}

static void copy_strings(
    void* mem,
    const char* cmd,
    size_t cmd_size,
    const char* const* args,
    const size_t* arg_sizes,
    size_t n_args
) {
    // Make some pointers
    char** new_args = (char**)(mem + sizeof(CmdStreamData));
    char* new_cmd = (char*)(new_args + n_args + 1);
    char* next_str = new_cmd + cmd_size + 1;
    // Initialize cmd
    strncpy(new_cmd, cmd, cmd_size);
    new_cmd[cmd_size] = 0;
    // Initialize args
    for (size_t i = 0; i < n_args; i++) {
        new_args[i] = next_str;
        next_str += arg_sizes[i] + 1;
        strncpy(new_args[i], args[i], arg_sizes[i]);
        new_args[i][arg_sizes[i]] = 0;
    }
    new_args[n_args] = NULL;
    // Set the data
    CmdStreamData* data = (CmdStreamData*)mem;
    data->cmd = new_cmd;
    data->args = (const char* const*)new_args;
    data->n_args = n_args;
}

void cmd_stream_new(
    const char* cmd,
    size_t cmd_size,
    const char* const* args,
    const size_t* arg_sizes,
    size_t n_args,
    Stream stdin_stream,
    CmdStream* stdout_stream,
    CmdStream* stderr_stream
) {
    void* mem = allocate_enough_memory(cmd_size, arg_sizes, n_args);
    CmdStreamData* data = (CmdStreamData*)mem;
    copy_strings(mem, cmd, cmd_size, args, arg_sizes, n_args);
    int stdin_read_fd = 0;
    open_feeder_child(stdin_stream, &data->feeder_child, &stdin_read_fd);
    int stdout_read_fd = 0;
    int stderr_read_fd = 0;
    open_cmd_child(
        stdin_read_fd,
        data->cmd,
        data->args,
        &data->cmd_child,
        &stdout_read_fd,
        &stderr_read_fd
    );
    data->ref_count = 2;
    // Finalize outputs
    stdout_stream->data = data;
    stdout_stream->kind = CS_KIND_STDOUT;
    stdout_stream->fd = stdout_read_fd;
    stderr_stream->data = data;
    stderr_stream->kind = CS_KIND_STDERR;
    stderr_stream->fd = stderr_read_fd;
}

void cmd_stream_free(CmdStream* this) {
    this->data->ref_count--;
    close(this->fd);
    if (this->data->ref_count == 0) {
        kill(this->data->feeder_child, SIGKILL);
        kill(this->data->cmd_child, SIGKILL);
        free(this->data);
    }
}

size_t cmd_stream_read(CmdStream* this, char* buffer, size_t size) {
    return read(this->fd, buffer, size);
}
