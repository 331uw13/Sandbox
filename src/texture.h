#ifndef SB_TEXTURE_H
#define SB_TEXTURE_H


// texture segment element count.
// x, y, red, grn, blu
#define SB_TEX_SEG_ELEMCOUNT 5

// number of elements for texture information
#define SB_TEX_INFO_ELEMCOUNT 4

#define SB_TEX_COLUMN_LIMIT 1024
#define SB_TEX_ROW_LIMIT 1024

#define SB_TEX_FILE_SIGNATURE 0x660AFF01


struct sb_texinfo_t {
    int sig;
    int cols;
    int rows;
    int pixels;
};

// takes open file descriptor must be at least opened with O_RDONLY.
// fills the structure with information
int sb_read_texinfo(struct sb_texinfo_t* ti, int fd);

struct texpx_t {
    int x;
    int y;
    float red;
    float grn;
    float blu;
};

struct texture_t {
    size_t num_pixels;
    struct texpx_t* data;
    size_t data_sizeb;
    int cols;
    int rows;
};

int sb_read_texf(const char* filepath, struct texture_t* tex);
void sb_delete_tex(struct texture_t* tex);



#endif
