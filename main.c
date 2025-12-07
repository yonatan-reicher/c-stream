#include "stream.h"

int main() {
    Stream s = STREAM_THEN_ALL(
        stream_str("Power now is "),
        stream_trim(stream_file("/sys/class/power_supply/BAT1/power_now")),
        stream_str(" 😌\n")
    );
    stream_read_all_to_file(s, stdout);
    const char* args[] = { "echo", "Hello", "World", NULL };
    Stream stdout_stream, stderr_stream;
    stream_cmd(
        "echo", args,
        stream_str("HIII\n"), &stdout_stream, &stderr_stream
    );
    stream_read_all_to_file(stdout_stream, stdout);
    stream_free(&stderr_stream);
    return 0;
}
