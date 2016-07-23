#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include <kclangc.h>

static KCDB *master_pgn_db;

void get_master_pgn(struct evhttp_request *req, void *context) {
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_error(req, HTTP_BADMETHOD, "Method Not Allowed");
        return;
    }

    const struct evhttp_uri *uri = evhttp_request_get_evhttp_uri(req);
    if (!uri) {
        puts("evhttp_request_get_evhttp_uri failed");
        return;
    }

    const char *path = evhttp_uri_get_path(uri);
    if (!path) {
        puts("evhttp_uri_get_path failed");
        return;
    }

    char game_id[9] = {};
    int end;
    if (1 != sscanf(path, "/master/pgn/%8s%n", game_id, &end) ||
            strlen(game_id) != 8 ||
            strlen(path) != end) {
        evhttp_send_error(req, HTTP_NOTFOUND, "Not Found");
        return;
    }

    size_t pgn_size;
    char *pgn = kcdbget(master_pgn_db, game_id, 8, &pgn_size);
    if (!pgn) {
        evhttp_send_error(req, HTTP_NOTFOUND, "Master PGN Not Found");
        return;
    }

    struct evbuffer *res = evbuffer_new();
    if (!res) {
        puts("could not allocate response buffer");
        abort();
    }

    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "application/vnd.chess-pgn; charset=utf-8");

    evbuffer_add(res, pgn, pgn_size);
    evhttp_send_reply(req, HTTP_OK, "OK", res);
    evbuffer_free(res);
}

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

    evhttp_set_gencb(http, get_master_pgn, NULL);  // master/pgn/{8}

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
    master_pgn_db = kcdbnew();
    if (!kcdbopen(master_pgn_db, "master-pgn.kct", KCOREADER)) {
        printf("master-pgn.kct open error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
        return 1;
    }

    int ret = serve(5555);

    if (!kcdbclose(master_pgn_db)) {
        printf("master-pgn.kct close error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
    }

    kcdbdel(master_pgn_db);
    return ret;
}