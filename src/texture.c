#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int sb_read_texf(const char* filepath, struct texture_t* tex) {
    int result = 0;
    int fd = open(filepath, O_RDONLY);
    int* tmp_data = NULL;

    if(!tex || !filepath) {
        goto error;
    }

    if(fd < 0) {
        fprintf(stderr, "(ERROR) %s: Failed to read texture from '%s'\n",
                __func__, filepath);
        goto error;
    }


    struct sb_texinfo_t texinfo;
    if(!sb_read_texinfo(&texinfo, fd)) {
        goto error_n_close;
    }

    if(texinfo.sig != SB_TEX_FILE_SIGNATURE) {
        fprintf(stderr, "(ERROR) %s: '%s' is not a texture file.\n",
                __func__, filepath);
        goto error_n_close;
    }

    if((texinfo.cols > SB_TEX_COLUMN_LIMIT)
    || (texinfo.rows > SB_TEX_ROW_LIMIT)) {
        fprintf(stderr, "(ERROR) %s: '%s' is too big.\n",
                __func__, filepath);
        goto error_n_close;
    }

    if(texinfo.pixels <= 0) {
        fprintf(stderr, "(ERROR) %s: '%s' has no data.\n",
                __func__, filepath);
        goto error_n_close;
    }



    tex->data_sizeb = texinfo.pixels * sizeof *tex->data;
    tex->data = NULL;
    tex->data = malloc(tex->data_sizeb);

    if(!tex->data) {
        fprintf(stderr, "(ERROR) %s: Failed to allocate %li bytes of memory for texture.\n",
                __func__, tex->data_sizeb);
        goto error_n_close;
    }
    

    const size_t tmp_data_sizeb = (texinfo.pixels * SB_TEX_SEG_ELEMCOUNT) * sizeof *tmp_data;
    tmp_data = malloc(tmp_data_sizeb);
    if(!tmp_data) {
        fprintf(stderr, "(ERROR) %s: Failed to allocate %li bytes of temporary data.\n",
                __func__, tmp_data_sizeb);
    }

    memset(tmp_data, 0, tmp_data_sizeb);

    if(read(fd, tmp_data, tmp_data_sizeb) < 0) {
        fprintf(stderr, "(ERROR) %s: Read failed to memory from texture file '%s'\n",
                __func__, filepath);
        goto error_n_close;
    }


    tex->cols = texinfo.cols;
    tex->rows = texinfo.rows;
    tex->num_pixels = 0;

    for(size_t i = 0;
            i < (size_t)(texinfo.pixels * SB_TEX_SEG_ELEMCOUNT);
                i += (SB_TEX_SEG_ELEMCOUNT))
    {
        int x = tmp_data[i];
        int y = tmp_data[i+1];
        float red = ((float)tmp_data[i+2]) / 255.0;
        float grn = ((float)tmp_data[i+3]) / 255.0;
        float blu = ((float)tmp_data[i+4]) / 255.0;

        if(tex->num_pixels >= (size_t)texinfo.pixels) {
            break;
        }

        struct texpx_t* tp = &tex->data[tex->num_pixels];
        if(!tp) { continue; }

        tp->x = x;
        tp->y = y;
        tp->red = red;
        tp->grn = grn;
        tp->blu = blu;

        tex->num_pixels++;
    }



error_n_close:

    if(tmp_data) {
        free(tmp_data);
    }
    
    close(fd);


error:
    return result;
}

void sb_delete_tex(struct texture_t* tex) {
    if(!tex) {
        return;
    }

    if(tex->data) {
        free(tex->data);
        tex->num_pixels = 0;
        tex->data_sizeb = 0;
    }

}

