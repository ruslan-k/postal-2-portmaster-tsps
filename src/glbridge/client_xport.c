#define _GNU_SOURCE
#include "xport.h"
#include "ops.h"
#include "postal2_frame.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define CMD_FLUSH (32u * 1024u)
#define XPORT_PATH "/tmp/tspgl-xport"
#define XPORT_MAGIC 0x58504f52u

static struct tspgl_shared *X;
static int xport_diag = -1;
static int real_glerror = -1;
static unsigned diag_calls;
static uint32_t recent_ops[16];
static unsigned recent_ops_count;
static unsigned recent_ops_pos;

static int xport_diag_enabled(void)
{
    const char *v;

    if (xport_diag >= 0)
        return xport_diag;
    v = getenv("POSTAL2_XPORT_DIAG");
    xport_diag = v && strcmp(v, "0") != 0;
    return xport_diag;
}

static int env_dim(const char *name, int fallback)
{
    const char *v = getenv(name);
    char *end = NULL;
    long n;
    if (!v || !*v) return fallback;
    n = strtol(v, &end, 10);
    if (!end || *end || n <= 0 || n > 8192) return fallback;
    return (int)n;
}

static int real_glerror_enabled(void)
{
    const char *v;

    if (real_glerror >= 0)
        return real_glerror;
    v = getenv("POSTAL2_REAL_GLERROR");
    real_glerror = v && strcmp(v, "0") != 0;
    return real_glerror;
}

struct tspgl_shared *tspgl_shared(void)
{
    int fd;
    int32_t pid;

    if (X)
        return X;
    fd = open(XPORT_PATH, O_RDWR | O_CREAT, 0600);
    if (fd < 0)
        return NULL;
    if (ftruncate(fd, (off_t)sizeof(*X)) != 0) {
        close(fd);
        return NULL;
    }
    X = mmap(NULL, sizeof(*X), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (X == MAP_FAILED) {
        X = NULL;
        return NULL;
    }
    pid = (int32_t)getpid();
    if (X->magic != XPORT_MAGIC || X->pid != pid) {
        if (__sync_bool_compare_and_swap(&X->lock, 0, 1)) {
            if (X->magic != XPORT_MAGIC || X->pid != pid) {
                memset(X, 0, sizeof(*X));
                X->lock = 1;
                X->sock = -1;
                X->pid = pid;
                X->unpack_align = 4;
                {
                    int w = env_dim("TSPGL_WIDTH", 1024);
                    int h = env_dim("TSPGL_HEIGHT", 768);
                    X->st_viewport[0] = 0;
                    X->st_viewport[1] = 0;
                    X->st_viewport[2] = w;
                    X->st_viewport[3] = h;
                    X->st_scissor[0] = 0;
                    X->st_scissor[1] = 0;
                    X->st_scissor[2] = w;
                    X->st_scissor[3] = h;
                    if (xport_diag_enabled())
                        fprintf(stderr, "P2-GEOM shadow-init viewport=0,0,%d,%d scissor=0,0,%d,%d\\n", w, h, w, h);
                }
                X->magic = XPORT_MAGIC;
                fprintf(stderr, "tspgl: shared xport pid=%d\n", (int)pid);
            }
            __sync_lock_release(&X->lock);
        } else {
            while (X->magic != XPORT_MAGIC || X->pid != pid)
                usleep(200);
        }
    }
    return X;
}

static void tspgl_lock(void)
{
    struct tspgl_shared *s = tspgl_shared();
    if (!s)
        return;
    while (__sync_lock_test_and_set(&s->lock, 1))
        ;
}

static void tspgl_unlock(void)
{
    struct tspgl_shared *s = tspgl_shared();
    if (s)
        __sync_lock_release(&s->lock);
}

static void tspgl_disconnect(void)
{
    struct tspgl_shared *s = tspgl_shared();
    if (!s)
        return;
    s->cmd_used = 0;
    if (s->sock >= 0) {
        close(s->sock);
        s->sock = -1;
    }
}

static int tspgl_connect(void)
{
    struct tspgl_shared *s = tspgl_shared();
    struct sockaddr_un addr;
    int fd;
    int n;

    if (!s)
        return -1;
    if (s->sock >= 0)
        return 0;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, TSPGL_SOCK, sizeof(addr.sun_path) - 1);
    for (n = 0; n < 50; ++n) {
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            int buf = 1024 * 1024;
            s->sock = fd;
            setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(buf));
            setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));
            fprintf(stderr, "tspgl: shared sock=%d\n", fd);
            return 0;
        }
        usleep(100000);
    }
    fprintf(stderr, "tspgl: connect %s errno=%d\n", TSPGL_SOCK, errno);
    close(fd);
    return -1;
}

static int full_write(int fd, const void *buf, size_t n)
{
    const uint8_t *p = buf;
    while (n) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += (size_t)w;
        n -= (size_t)w;
    }
    return 0;
}

static int full_read(int fd, void *buf, size_t n)
{
    uint8_t *p = buf;
    while (n) {
        ssize_t r = read(fd, p, n);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (r == 0)
            return -1;
        p += (size_t)r;
        n -= (size_t)r;
    }
    return 0;
}

static int tspgl_flush_locked(void)
{
    struct tspgl_shared *s = tspgl_shared();
    if (!s || !s->cmd_used)
        return 0;
    if (full_write(s->sock, s->cmdbuf, s->cmd_used) != 0) {
        s->cmd_used = 0;
        return -1;
    }
    s->cmd_used = 0;
    return 0;
}

int tspgl_call(uint32_t op, const void *in, uint32_t in_len, void *out,
               uint32_t out_cap)
{
    struct tspgl_shared *s;
    struct tspgl_hdr h, r;
    static uint8_t scratch_s[2 * 1024 * 1024];
    uint8_t *scratch = scratch_s;
    uint8_t *scratch_h = NULL;
    uint32_t seq = 0;
    int attempt;
    int rc = -1;
    int want_reply;
    int diag_active = 0;
    int diag_get_integerv = 0;
    int reply_read;
    uint32_t diag_pname = 0;
    uint32_t need;

    if (op == OP_glGetError && !real_glerror_enabled()) {
        if (out && out_cap >= 4)
            memset(out, 0, 4);
        return 0;
    }

    s = tspgl_shared();
    if (!s)
        return -1;
    want_reply = tspgl_needs_reply(op);
    if (xport_diag_enabled() && op != OP_glGetError) {
        recent_ops[recent_ops_pos++ & 15u] = op;
        if (recent_ops_count < 16)
            recent_ops_count++;
    }
    need = (uint32_t)sizeof(h) + in_len;
    if (want_reply && xport_diag_enabled() && diag_calls < 32) {
        diag_active = 1;
        diag_calls++;
        if (op == OP_glGetIntegerv && in && in_len >= 4) {
            memcpy(&diag_pname, in, 4);
            diag_get_integerv = 1;
            fprintf(stderr, "P2-XPORT glGetIntegerv begin pname=0x%08x\n",
                    diag_pname);
        }
        fprintf(stderr,
                "P2-XPORT enter op=%u seq=%u in_len=%u\n",
                op, s->seq + 1u, in_len);
    }
    tspgl_lock();
    if (diag_active)
        fprintf(stderr, "P2-XPORT lock-acquired op=%u\n", op);
    for (attempt = 0; attempt < 2; ++attempt) {
        if (tspgl_connect() != 0)
            break;
        seq = ++s->seq;
        h.magic = TSPGL_MAGIC;
        h.op = op;
        h.seq = seq;
        h.len = in_len;

        if (!want_reply && need <= TSPGL_CMD_CAP) {
            if (s->cmd_used + need > TSPGL_CMD_CAP && tspgl_flush_locked() != 0) {
                if (s->fail_log < 64) {
                    fprintf(stderr,
                            "tspgl: flush op=%u retry=%d errno=%d\n", op,
                            attempt, errno);
                    s->fail_log++;
                }
                tspgl_disconnect();
                continue;
            }
            memcpy(s->cmdbuf + s->cmd_used, &h, sizeof(h));
            s->cmd_used += (uint32_t)sizeof(h);
            if (in_len) {
                memcpy(s->cmdbuf + s->cmd_used, in, in_len);
                s->cmd_used += in_len;
            }
            if (s->cmd_used >= CMD_FLUSH || op == OP_glFlush) {
                if (tspgl_flush_locked() != 0) {
                    tspgl_disconnect();
                    continue;
                }
            }
            rc = 0;
            break;
        }

        if (tspgl_flush_locked() != 0 ||
            full_write(s->sock, &h, sizeof(h)) != 0 ||
            (in_len && full_write(s->sock, in, in_len) != 0)) {
            if (s->fail_log < 64) {
                fprintf(stderr, "tspgl: call op=%u retry=%d errno=%d in_len=%u\n",
                        op, attempt, errno, in_len);
                s->fail_log++;
            }
            tspgl_disconnect();
            continue;
        }
        if (diag_active)
            fprintf(stderr, "P2-XPORT sent op=%u seq=%u\n", op, seq);
        if (!want_reply) {
            rc = 0;
            break;
        }
        reply_read = full_read(s->sock, &r, sizeof(r));
        if (reply_read == 0 && diag_active)
            fprintf(stderr,
                    "P2-XPORT reply-hdr op=%u seq=%u status=%u len=%u\n",
                    op, r.seq, r.op, r.len);
        if (reply_read != 0 || r.magic != TSPGL_MAGIC || r.seq != seq ||
            r.len > TSPGL_MAX_BLOB) {
            if (s->fail_log < 64) {
                fprintf(stderr, "tspgl: reply op=%u retry=%d errno=%d\n", op,
                        attempt, errno);
                s->fail_log++;
            }
            tspgl_disconnect();
            continue;
        }
        if (r.len) {
            if (r.len > sizeof(scratch_s)) {
                scratch_h = malloc(r.len);
                if (!scratch_h) {
                    tspgl_disconnect();
                    continue;
                }
                scratch = scratch_h;
            }
            if (full_read(s->sock, scratch, r.len) != 0) {
                free(scratch_h);
                scratch_h = NULL;
                scratch = scratch_s;
                tspgl_disconnect();
                continue;
            }
            if (out && out_cap) {
                uint32_t n = r.len < out_cap ? r.len : out_cap;
                memcpy(out, scratch, n);
            }
        }
        rc = r.op == TSPGL_OK ? 0 : -1;
        break;
    }
    if (op == OP_glGetError && xport_diag_enabled() && rc == 0 && out && out_cap >= 4) {
        uint32_t error_value = 0;
        unsigned i;
        memcpy(&error_value, out, 4);
        if (error_value != 0) {
            fprintf(stderr, "P2-XPORT glGetError value=0x%04x recent=", error_value);
            for (i = 0; i < recent_ops_count; ++i) {
                unsigned idx = (recent_ops_pos + 16u - recent_ops_count + i) & 15u;
                fprintf(stderr, "%u%s", recent_ops[idx], i + 1 == recent_ops_count ? "" : ",");
            }
            fprintf(stderr, "\\n");
        }
    }
    if (diag_active) {
        if (diag_get_integerv) {
            int32_t value = 0;

            if (rc == 0 && out && out_cap >= 4)
                memcpy(&value, out, 4);
            fprintf(stderr,
                    "P2-XPORT glGetIntegerv end pname=0x%08x value=0x%08x rc=%d\n",
                    diag_pname, (uint32_t)value, rc);
        }
        fprintf(stderr, "P2-XPORT end op=%u seq=%u rc=%d\n", op, seq, rc);
    }
    free(scratch_h);
    tspgl_unlock();
    return rc;
}

/*
 * 32-bit runtime dlsyms this from libEGL. The 64-bit presenter reads the
 * same /tmp/postal2.frame header and draws the crosshair after the FBO blit.
 */
__attribute__((used, visibility("default")))
void postal2_fb_set_cursor(int x, int y, int on)
{
    static uint32_t *hdr;

    if (hdr == NULL) {
        int fd = open(POSTAL2_FRAME_PATH, O_RDWR);
        void *map;

        if (fd < 0)
            return;
        map = mmap(NULL, POSTAL2_FRAME_HDR, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
        close(fd);
        if (map == MAP_FAILED)
            return;
        hdr = map;
    }
    hdr[POSTAL2_HDR_CURSOR_X] = (uint32_t)x;
    hdr[POSTAL2_HDR_CURSOR_Y] = (uint32_t)y;
    hdr[POSTAL2_HDR_CURSOR_ON] = on != 0 ? 1u : 0u;
}
