#define _GNU_SOURCE

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>

#include "rooms.h"


/* program name */
char *progname;


/* main function */
int main(int argc, char **argv) {
    progname = basename(argv[0]);

    char *room_dir = NULL, **room_names = NULL;
    struct room *rooms = NULL;

    int room_idx, room_name_idx, conn_idx;

    int ret = 1;

    /* create room directory */
    if (create_room_dir(&room_dir) == -1)
        return 1;

    /* parse room names */
    int num_room_names = read_room_names(&room_names);
    if (num_room_names == -1)
        goto cleanup1;

    /* create rooms */
    if (create_rooms(room_dir, room_names, num_room_names) == -1)
        goto cleanup2;

    /* read rooms in again */
    int num_rooms;
    if ((num_rooms = read_rooms(room_dir, &rooms)) == -1)
        goto cleanup3;

    ret = 0;

    /* start readline loop */
    struct room *current_room, *next_room;

    for (room_idx = 0; room_idx < NUM_ROOMS; ++room_idx) {
        if (rooms[room_idx].type == START_ROOM)
            current_room = &rooms[room_idx];
    }

    char const *prompt_fmt =
        "CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: %s\nWHERE TO? >";

    char connections[100];

    for (;;) {
        connections[0] = '\0';
        for (conn_idx = 0; conn_idx < MAX_CONN; ++conn_idx) {
            if (!(next_room = current_room->connections[conn_idx]))
                break;

            strcat(connections, next_room->name);

            if (conn_idx < MAX_CONN - 1
                && current_room->connections[conn_idx + 1]) {

                strcat(connections, ", ");
            }
        }

        char *prompt;
        asprintf(&prompt, prompt_fmt, current_room->name, connections);

        char *buf = readline(prompt);

        free(prompt);

        if (!buf)
            break;

        putchar('\n');

        int next_room_valid = 0;
        for (conn_idx = 0; conn_idx < MAX_CONN; ++conn_idx) {
            if (!(next_room = current_room->connections[conn_idx]))
                break;

            if (strcmp(buf, next_room->name) == 0) {
                next_room_valid = 1;
                break;
            }
        }

        free(buf);

        if (next_room_valid) {
            current_room = next_room;
            if (current_room->type == END_ROOM) {
                printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                /* TODO: display path */
                break;
            }
        } else {
            printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        }
    }

    putchar('\n');

cleanup3:
    for (room_idx = 0; room_idx < num_rooms; ++room_idx)
        free(rooms[room_idx].name);

    free(rooms);

cleanup2:
    for (room_name_idx = 0; room_name_idx < num_room_names; ++room_name_idx)
        free(room_names[room_name_idx]);

    free(room_names);

cleanup1:
    free(room_dir);

    return ret;
}
