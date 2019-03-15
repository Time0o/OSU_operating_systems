#include <libgen.h>
#include <stdlib.h>

#include "rooms.h"


/* program name */
char *progname;


/* main function */
int main(int argc, char **argv) {
    progname = basename(argv[0]);

    /* create room directory */
    char *room_dir;
    if (create_room_dir(&room_dir) == -1)
        return -1;

    /* parse room names */
    char **room_names;
    int num_room_names = parse_room_names(&room_names);
    if (num_room_names == -1)
        return -1;

    /* create rooms */
    if (create_rooms(room_dir, room_names, num_room_names) == -1)
        return -1;

    /* free resources */
    free(room_dir);

    int room_name_idx = 0;
    for (room_name_idx = 0; room_name_idx < num_room_names; ++room_name_idx)
        free(room_names[room_name_idx]);

    free(room_names);

    return 0;
}
