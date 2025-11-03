#include "stream.h"

int main() {
    Stream a[] = {
        stream_str("Power now is "),
        stream_trim(stream_file("/sys/class/power_supply/BAT1/power_now")),
        stream_str(" 😌\n"),
    };
    Stream s = stream_then_all(a, 3);
    stream_read_all_to_file(&s, stdout);
    stream_free(&s);
    const char* args[] = { "cat", NULL };
    Stream stdout_stream, stderr_stream;
    stream_cmd(
        "cat", args,
        stream_str("HIII\n"), &stdout_stream, &stderr_stream
    );
    stream_read_all_to_file(&stdout_stream, stdout);
    stream_free(&stdout_stream);
    stream_free(&stderr_stream);
    return 0;
}
