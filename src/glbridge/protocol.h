#ifndef TSP_GLBRIDGE_PROTOCOL_H
#define TSP_GLBRIDGE_PROTOCOL_H

#include <stdint.h>

#define TSPGL_SOCK "/tmp/tsp-glbridge.sock"
#define TSPGL_MAGIC 0x47333236u /* G326 */
#define TSPGL_MAX_BLOB (64u * 1024u * 1024u)

struct tspgl_hdr {
    uint32_t magic;
    uint32_t op;
    uint32_t seq;
    uint32_t len;
};

enum {
    TSPGL_OK = 0,
    TSPGL_ERR = 1
};

/* Extra ops not in the generated GLES table. */
enum {
    OP_EGL_SWAP = 1,
    OP_PING = 2,
    OP_UNKNOWN_NAME = 3,
    OP_GLES_BASE = 16
};

#endif
