#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include <kclangc.h>

#include "attacks.h"
#include "board.h"
#include "encode.h"
#include "pgn.h"

static KCDB *master_pgn_db;
static KCDB *master_db;

static bool cors = true;
static bool verbose = true;

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
    kcfree(pgn);

    evhttp_send_reply(req, HTTP_OK, "OK", res);
    evbuffer_free(res);
}

void get_master(struct evhttp_request *req, void *context) {
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_error(req, HTTP_BADMETHOD, "Method Not Allowed");
        return;
    }

    const char *uri = evhttp_request_get_uri(req);
    if (!uri) {
        puts("evhttp_request_get_uri failed");
        return;
    }

    struct evkeyvalq query;
    const char *fen = NULL;
    const char *jsonp = NULL;
    int moves = 12;
    int topGames = 4;
    if (0 == evhttp_parse_query(uri, &query)) {
        fen = evhttp_find_header(&query, "fen");
        jsonp = evhttp_find_header(&query, "callback");

        if (evhttp_find_header(&query, "moves")) {
            moves = atoi(evhttp_find_header(&query, "moves"));
        }

        if (evhttp_find_header(&query, "topGames")) {
            topGames = atoi(evhttp_find_header(&query, "topGames"));
        }
    }
    if (!fen || !strlen(fen)) {
        evhttp_send_error(req, HTTP_BADREQUEST, "Missing FEN");
        return;
    }

    // Look up positon.
    board_t pos;
    if (!board_set_fen(&pos, fen)) {
        evhttp_send_error(req, HTTP_BADREQUEST, "Invalid FEN");
    }

    if (verbose) printf("master: %.255s\n", fen);

    struct evbuffer *res = evbuffer_new();
    if (!res) {
        puts("could not allocate response buffer");
        abort();
    }

    // CORS.
    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    if (cors) evhttp_add_header(headers, "Access-Control-Allow-Origin", "*");

    // Set Content-Type.
    if (jsonp && strlen(jsonp)) {
        evhttp_add_header(headers, "Content-Type", "application/javascript");
        evbuffer_add_printf(res, "%s(", jsonp);
    } else {
        evhttp_add_header(headers, "Content-Type", "application/json");
    }

    uint64_t zobrist_hash = board_zobrist_hash(&pos, POLYGLOT);

    struct master_record *record = master_record_new();
    size_t record_size;
    char *encoded_record = kcdbget(master_db, (const char *) &zobrist_hash, 8, &record_size);
    if (encoded_record) {
        decode_master_record((const uint8_t *) encoded_record, record);
    }

    unsigned long average_rating_sum = master_record_average_rating_sum(record);
    unsigned long total_white = master_record_white(record);
    unsigned long total_draws = master_record_draws(record);
    unsigned long total_black = master_record_black(record);
    unsigned long total = total_white + total_draws + total_black;

    evbuffer_add_printf(res, "{\n");

    // Add totals.
    evbuffer_add_printf(res, "  \"white\": %lu,\n", total_white);
    evbuffer_add_printf(res, "  \"draws\": %lu,\n", total_draws);
    evbuffer_add_printf(res, "  \"black\": %lu,\n", total_black);
    if (total) evbuffer_add_printf(res, "  \"averageRating\": %lu,\n", average_rating_sum / total);
    else evbuffer_add_printf(res, "  \"averageRating\": null,\n");

    // Add move list.
    evbuffer_add_printf(res, "  \"moves\": [\n");
    for (size_t i = 0; i < record->num_moves && i < moves; i++) {
        unsigned long move_total = record->moves[i].white + record->moves[i].draws + record->moves[i].black;

        char uci[LEN_UCI], san[LEN_SAN];
        move_uci(record->moves[i].move, uci);
        board_san(&pos, record->moves[i].move, san);

        evbuffer_add_printf(res, "    {\n");
        evbuffer_add_printf(res, "      \"uci\": \"%s\",\n", uci);
        evbuffer_add_printf(res, "      \"san\": \"%s\",\n", san);
        evbuffer_add_printf(res, "      \"white\": %lu,\n", record->moves[i].white);
        evbuffer_add_printf(res, "      \"draws\": %lu,\n", record->moves[i].draws);
        evbuffer_add_printf(res, "      \"black\": %lu,\n", record->moves[i].black);
        if (move_total) {
            evbuffer_add_printf(res, "      \"averageRating\": %lu\n", record->moves[i].average_rating_sum / move_total);
        } else {
            evbuffer_add_printf(res, "      \"averageRating\": null\n");
        }
        evbuffer_add_printf(res, "    }%s\n", (i < record->num_moves - 1 && i < moves - 1) ? "," : "");
    }

    evbuffer_add_printf(res, "  ],\n");

    // Add top games.
    evbuffer_add_printf(res, "  \"topGames\": [\n");
    for (size_t i = 0; i < record->num_refs && i < topGames; i++) {
        char game_id[9];
        strncpy(game_id, record->refs[i].game_id, 8);

        size_t pgn_size;
        char *pgn = kcdbget(master_pgn_db, game_id, 8, &pgn_size);
        if (!pgn) continue;

        char *save_ptr;
        struct pgn_game_info *game_info = pgn_game_info_read(pgn, &save_ptr);
        if (!game_info->white || !game_info->black) continue;

        evbuffer_add_printf(res, "    {\n");
        // TODO: winner, white.name, white.rating, black.name, black.rating, -avg rating
        evbuffer_add_printf(res, "      \"id\": \"%s\",\n", game_id);
        evbuffer_add_printf(res, "      \"winner\": \"%s\",\n", game_info->result ? (game_info->result > 0 ? "white" : "black") : "draw");
        evbuffer_add_printf(res, "      \"white\": {\n");
        evbuffer_add_printf(res, "        \"name\": \"%s\",\n", game_info->white); // XXX TODO !
        evbuffer_add_printf(res, "        \"rating\": %d\n", game_info->white_elo);
        evbuffer_add_printf(res, "      },\n");
        evbuffer_add_printf(res, "      \"black\": {\n");
        evbuffer_add_printf(res, "        \"name\": \"%s\",\n", game_info->black); // XXX TODO !
        evbuffer_add_printf(res, "        \"rating\": %d\n", game_info->black_elo);
        evbuffer_add_printf(res, "      },\n");
        evbuffer_add_printf(res, "      \"year\": %d\n", game_info->year);
        evbuffer_add_printf(res, "    }%s\n", (i < record->num_refs - 1 && i < topGames - 1) ? "," : "");
    }
    evbuffer_add_printf(res, "  ]\n");

    evbuffer_add_printf(res, "}%s\n", (jsonp && strlen(jsonp)) ? ")" : "");

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

    evhttp_set_cb(http, "/master", get_master, NULL); // master
    evhttp_set_gencb(http, get_master_pgn, NULL);     // master/pgn/{8}

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
    attacks_init();

    master_pgn_db = kcdbnew();
    puts("opening master-pgn.kct ...");
    if (!kcdbopen(master_pgn_db, "master-pgn.kct", KCOREADER)) {
        printf("master-pgn.kct open error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
        return 1;
    }

    master_db = kcdbnew();
    puts("opening master.kch ...");
    if (!kcdbopen(master_db, "master.kch", KCOREADER)) {
        printf("master.kch open error: %s\n", kcecodename(kcdbecode(master_db)));
        return 1;
    }

    puts("opened all databases.");

    int ret = serve(5555);

    if (!kcdbclose(master_pgn_db)) {
        printf("master-pgn.kct close error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
    }

    if (!kcdbclose(master_db)) {
        printf("master.kch close error: %s\n", kcecodename(kcdbecode(master_db)));
    }

    kcdbdel(master_pgn_db);
    return ret;
}
