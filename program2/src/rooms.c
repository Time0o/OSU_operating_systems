#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "rooms.h"
#include "util.h"


static char const *ROOM_DIR_FMT = "%s.rooms.%ju";
static char const *ROOM_NAME_FILE = "../resources/room_names.txt";
char const *ROOM_TYPE_STR[] = { ROOM_TYPES(STRING) };


int parse_room_names(char ***room_names) {
    /* open room name file */
    char *source_file = malloc(strlen(__FILE__) + 1);
    strcpy(source_file, __FILE__);

    char *room_name_path;
    if (asprintf(&room_name_path, "%s/%s", dirname(source_file), ROOM_NAME_FILE) == -1) {
        errprintf("asprintf failed");
        free(source_file);
        return -1;
    }

    free(source_file);

    FILE *fp;
    if (!(fp = fopen(room_name_path, "r"))) {
        errprintf("failed to open '%s'", room_name_path);
        free(room_name_path);
        return -1;
    }

    free(room_name_path);

    /* count number of distinct room names */
    int num_room_names = 0;

    for (;;) {
        char ch = fgetc(fp);

        if (ch == EOF) {
            if (ferror(fp)) {
                errprintf("fgetc failed");
                fclose(fp);
                return -1;

            } else {
                break;
            }

        } else if (ch == '\n') {
            ++num_room_names;
        }
    }

    rewind(fp);

    assert(num_room_names >= NUM_ROOMS);

    /* read in room names */
    *room_names = malloc(num_room_names * sizeof(char *));
    int num_room_names_valid = 0;

    int r = 0;
    for (r = 0; r < num_room_names; ++r) {
        char *line = NULL;
        size_t len = 0;

        errno = 0;
        if (getline(&line, &len, fp) == -1) {
            if (errno == EINVAL || errno == ENOMEM) {
                errprintf("getline failed");

                int r_ = 0;
                for (r_ = 0; r_ < num_room_names_valid; ++r_)
                    free((*room_names)[r_]);

                free((*room_names));

                fclose(fp);

                return -1;

            } else {
                break;
            }
        }

        line[strlen(line) - 1] = '\0';
        (*room_names)[r] = line;

        ++num_room_names_valid;
    }

    fclose(fp);

    return num_room_names;
}


int create_room_dir(char **room_dir) {
    char *username = getlogin();
    pid_t pid = getpid();

    if (asprintf(room_dir, ROOM_DIR_FMT, username, (uintmax_t) pid) == -1) {
        errprintf("asprintf failed");
        return -1;
    }

    struct stat st;
    if (stat(*room_dir, &st) == 0) {
        errprintf("file '%s' already exists", *room_dir);
        free(*room_dir);
        return -1;

    } else if (errno != ENOENT) {
        errprintf("failed to stat file '%s'", *room_dir);
        free(*room_dir);
        return -1;
    }

    if (mkdir(*room_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        errprintf("failed to create directory '%s'", *room_dir);
        free(*room_dir);
        return -1;
    }

    return 0;
}


int create_rooms(char *room_dir, char **room_names, int num_room_names) {
    /* see rand */
    srand(time(NULL));

    /* indexing variables */
    int room_idx, room_idx_next, room_name_idx, conn, conn_prev, conn_next;

    /* create rooms structures */
    struct room *rooms = malloc(NUM_ROOMS * sizeof(struct room));
    int *room_name_taken = calloc(num_room_names, sizeof(int));

    for (room_idx = 0; room_idx < NUM_ROOMS; ++room_idx) {
        struct room room;

        /* randomly choose next room name */
        do {
            room_name_idx = rand() % num_room_names;
        } while (room_name_taken[room_name_idx]);

        room.name = room_names[room_name_idx];
        room_name_taken[room_name_idx] = 1;

        /* pre-allocate connection array */
        int conn = 0;
        for (conn = 0; conn < MAX_CONN; ++conn)
            room.connections[conn] = NULL;

        /* set room type */
        if (room_idx == 0)
            room.type = START_ROOM;
        else if (room_idx == NUM_ROOMS - 1)
            room.type = END_ROOM;
        else
            room.type = MID_ROOM;

        rooms[room_idx] = room;
    }

    free(room_name_taken);

    /* connect rooms */
    for (room_idx = 0; room_idx < NUM_ROOMS - 1; ++room_idx) {
        struct room *room = &rooms[room_idx];

        int num_connections = rand() % (MAX_CONN - MIN_CONN) + MIN_CONN;

        for (conn = 0; conn < num_connections; ++conn) {
            if (room->connections[conn])
                continue;

            for (;;) {
                room_idx_next = rand() % NUM_ROOMS;

                if (room_idx_next == room_idx)
                    continue;

                struct room *next_room = &rooms[room_idx_next];

                int unique = 1;
                for (conn_prev = conn - 1; conn_prev >= 0; --conn_prev) {
                    if (room->connections[conn_prev] == next_room) {
                        unique = 0;
                        break;
                    }
                }

                if (!unique)
                    continue;

                conn_next = 0;
                while (next_room->connections[conn_next])
                    ++conn_next;

                next_room->connections[conn_next] = room;

                break;
            }

            room->connections[conn] = &rooms[room_idx_next];
        }
    }

    int ret = write_rooms(room_dir, rooms);

    free(rooms);

    return ret;
}


static void write_room(struct room room, FILE *fp) {
    fprintf(fp, "ROOM NAME: %s\n", room.name);

    int conn = 0;
    for (;;) {
        struct room *next_room = room.connections[conn];
        if (!next_room)
            break;

        fprintf(fp, "CONNECTION %d: %s\n", conn + 1, next_room->name);

        if (++conn >= MAX_CONN)
            break;
    }

    fprintf(fp, "ROOM_TYPE: %s\n", ROOM_TYPE_STR[room.type]);
}


int write_rooms(char *room_dir, struct room *rooms) {
    int room_idx = 0;
    for (room_idx = 0; room_idx < NUM_ROOMS; ++room_idx) {
        struct room room = rooms[room_idx];

        char *file_name = NULL;
        if (asprintf(&file_name, "%s/%s.txt", room_dir, room.name) == -1) {
            errprintf("asprintf failed");
            return -1;
        }

        FILE *fp;
        if ((fp = fopen(file_name, "w")) == NULL) {
            errprintf("failed to open '%s'", file_name);
            free(file_name);
            return -1;
        }

        free(file_name);

        write_room(room, fp);

        fclose(fp);
    }

    return 0;
}