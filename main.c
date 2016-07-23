#include <stdio.h>
#include <stdlib.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

int serve(int port) {
    struct event_base *base = event_base_new();
    if (!base) {
        puts("could not initialize event_base");
        abort();
    }

    struct evhttp *http = evhttp_new(base);
    if (!http) {
        puts("could not initialize evhttp");
        abort();
    }

    struct evhttp_bound_socket *socket =
        evhttp_bind_socket_with_handle(http, "127.0.0.1", port);
    if (!socket) {
        printf("could not bind socket to http://127.0.0.1:%d/\n", port);
        return 1;
    }

    printf("listening on http://127.0.0.1:%d/ ...\n", port);

    return event_base_dispatch(base);
}

int main() {
    return serve(5555);
}
