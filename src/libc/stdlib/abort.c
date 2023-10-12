#include <stdlib.h>
#include <env.h>

void abort(void) {
    trace_put_char('[');
    trace_put_char('A');
    trace_put_char('b');
    trace_put_char('o');
    trace_put_char('r');
    trace_put_char('t');
    trace_put_char(']');
    trace_put_char('\n');
    system_exit();
    // wasm doesn't support signals, so just trap to halt the program.
    __builtin_trap();
}

void exit(int status) {
    // just ignore the status
    system_exit();
    __builtin_trap();
}
