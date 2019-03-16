#include <libgen.h>
#include <stdlib.h>

#include "rooms.h"


/* program name */
char *progname;


/* main function */
int main(int argc, char **argv) {
    progname = basename(argv[0]);

    char *room_dir = NULL, **room_names = NULL;
    struct room *rooms = NULL;

    int room_idx, room_name_idx, ret = 1;

    /* create room directory */
    if (create_room_dir(&room_dir) == -1)
        return 1;

    /* parse room names */
    int num_room_names = parse_room_names(&room_names);
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
