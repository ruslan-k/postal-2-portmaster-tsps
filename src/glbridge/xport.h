#ifndef TSPGL_XPORT_H
#define TSPGL_XPORT_H

#include "protocol.h"
#include <stdint.h>
#include <string.h>

#define TSPGL_CMD_CAP (256u * 1024u)

/* One mmap for all copies of this .so (libEGL + libGLESv2 + SONAME
 * aliases). FAT32 has no symlinks, so the game loads four DSOs with
 * four static tspgl_fd's — four sockets into one GPU context, draws
 * beat texture uploads, road tiles stay black. */
struct tspgl_shared {
    uint32_t magic;
    int32_t pid;
    int lock;
    int sock;
    uint32_t seq;
    int fail_log;
    uint32_t cmd_used;
    uint8_t cmdbuf[TSPGL_CMD_CAP];
    unsigned active_tex;
    uint32_t tex_bind_2d[32];
    uint32_t tex_bind_cube[32];
    uint32_t bound_fb;
    uint32_t bound_read_fb;
    uint32_t bound_rb;
    uint32_t bound_array;
    uint32_t bound_element;
    uint32_t bound_vao;
    uint32_t cur_prog;
    int32_t st_viewport[4];
    int32_t st_scissor[4];
    int unpack_align;
};

static inline uint32_t tspgl_pack_f32(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

static inline float tspgl_unpack_f32(uint32_t u)
{
    float f;
    memcpy(&f, &u, 4);
    return f;
}

struct tspgl_shared *tspgl_shared(void);
int tspgl_call(uint32_t op, const void *in, uint32_t in_len, void *out,
               uint32_t out_cap);

#endif
