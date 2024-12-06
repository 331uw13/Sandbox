#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "texture.h"

int sb_read_texinfo(struct sb_texinfo_t* ti, int fd) {
    int result = 0;

    if(read(fd, ti, sizeof *ti) < 0) {
        fprintf(stderr, 
                "(ERROR) %s: %s\n"
                "Failed to read first %li bytes for info about texture file\n",
                __func__, strerror(errno), sizeof *ti);
        goto error;
    }

    result = 1;

error:
    return result;
}



