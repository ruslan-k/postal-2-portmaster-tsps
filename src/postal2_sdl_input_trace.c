/* i386 SDL 1.2 classifier with a passive shared-frame cursor marker. */
/* No 32-bit development headers are required to cross-build this probe. */
#include "postal2_frame.h"
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef struct _IO_FILE FILE;
extern FILE *stderr;
extern int fprintf(FILE *, const char *, ...);
extern void *dlsym(void *, const char *);
extern char *getenv(const char *);
extern int open(const char *, int, ...);
extern void *mmap(void *, unsigned long, int, int, int, long);
extern int close(int);
extern int munmap(void *, unsigned long);
#define RTLD_NEXT ((void *)-1)
#ifndef O_RDWR
#define O_RDWR 2
#endif
#ifndef PROT_READ
#define PROT_READ 1
#endif
#ifndef PROT_WRITE
#define PROT_WRITE 2
#endif
#ifndef MAP_SHARED
#define MAP_SHARED 1
#endif
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned char SDL_Event[24];
static unsigned records;
static uint16_t last_x, last_y;
static int have_position;
static unsigned peep_gets;
static unsigned cursor_records;
static unsigned mouse_state_records;
static unsigned diag_cursor_records;
static unsigned diag_cursor_map_attempts;
static volatile uint32_t *diag_cursor_header;
static int mode_is(const char *wanted) {
    const char *mode = getenv("POSTAL2_MOUSE_COORD_MODE");
    if (!mode) return 0;
    while (*wanted && *mode && *wanted == *mode) { ++wanted; ++mode; }
    return !*wanted && !*mode;
}
static int env_is_one(const char *name) {
    const char *value = getenv(name);
    return value && value[0] == '1' && value[1] == '\0';
}
static int cursor_event_position(const SDL_Event *event, int *x, int *y) {
    if (!event || !x || !y) return 0;
    unsigned type = (*event)[0];
    if (type != 4 && type != 5 && type != 6) return 0;
    const uint16_t *xy = (const uint16_t *)(const void *)(*event + 4);
    *x = (int)xy[0];
    *y = (int)xy[1];
    return 1;
}
static volatile uint32_t *map_cursor_frame(const char *path) {
    int fd;
    void *map;
    volatile uint32_t *hdr;

    if (!path || !*path) return NULL;
    fd = open(path, O_RDWR);
    if (fd < 0) return NULL;
    map = mmap(NULL, POSTAL2_FRAME_HDR, PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return NULL;
    hdr = (volatile uint32_t *)map;
    if (hdr[POSTAL2_HDR_MAGIC] != POSTAL2_FRAME_MAGIC) {
        munmap(map, POSTAL2_FRAME_HDR);
        return NULL;
    }
    return hdr;
}

static int store_cursor_frame_header(volatile uint32_t *hdr, int x, int y, int on) {
    if (!hdr || hdr[POSTAL2_HDR_MAGIC] != POSTAL2_FRAME_MAGIC ||
        x < 0 || y < 0 || x >= POSTAL2_FRAME_MAX_W || y >= POSTAL2_FRAME_MAX_H)
        return 0;
    hdr[POSTAL2_HDR_CURSOR_X] = (uint32_t)x;
    hdr[POSTAL2_HDR_CURSOR_Y] = (uint32_t)y;
    hdr[POSTAL2_HDR_CURSOR_ON] = on != 0 ? 1u : 0u;
    return 1;
}

static int publish_cursor_event(volatile uint32_t *hdr, SDL_Event *event,
                                int *x_out, int *y_out) {
    int x, y;
    if (!cursor_event_position((const SDL_Event *)event, &x, &y)) return 0;
    if (!store_cursor_frame_header(hdr, x, y, 1)) return 0;
    if (x_out) *x_out = x;
    if (y_out) *y_out = y;
    return 1;
}

static void publish_cursor(SDL_Event *event) {
    int x, y;
    unsigned type;
    if (!event || !env_is_one("POSTAL2_DIAG_UWINDOW_CURSOR")) return;
    type = (*event)[0];
    if (type != 4 && type != 5 && type != 6) return;
    if (!diag_cursor_header) {
        if (diag_cursor_map_attempts >= 3) return;
        ++diag_cursor_map_attempts;
        diag_cursor_header = map_cursor_frame(POSTAL2_FRAME_PATH);
        if (!diag_cursor_header) {
            fprintf(stderr, "P2-MAP frame_header=unavailable attempt=%u\n",
                    diag_cursor_map_attempts);
            return;
        }
        fprintf(stderr, "P2-MAP frame_header=ready path=%s\n", POSTAL2_FRAME_PATH);
    }
    if (!publish_cursor_event(diag_cursor_header, event, &x, &y)) return;
    if (diag_cursor_records < 120) {
        if (type == 4) {
            const int16_t *rel = (const int16_t *)(const void *)(*event + 8);
            fprintf(stderr, "P2-MAP source=raw-sdl-xy event=%u guest=%d,%d rel=%d,%d\n",
                    type, x, y, rel[0], rel[1]);
        } else {
            fprintf(stderr, "P2-MAP source=raw-sdl-xy event=%u guest=%d,%d\n",
                    type, x, y);
        }
    }
    ++diag_cursor_records;
}

static int force_cursor_toggle(int requested) {
    const char *force = getenv("POSTAL2_FORCE_CURSOR");
    if (requested == 0 && force && force[0] == '1' && force[1] == '\0')
        return 1;
    return requested;
}
static int force_grab_mode(int requested) {
    const char *force = getenv("POSTAL2_FORCE_UNGRAB");
    if (requested == 1 && force && force[0] == '1' && force[1] == '\0')
        return 0;
    return requested;
}
static void normalize(SDL_Event *event) {
    if (!event) return;
    int absolute = mode_is("absolute");
    int relative_mode = mode_is("relative");
    /* SDL 1.2 already defines x/y as absolute window coordinates and
     * xrel/yrel as motion deltas. Keep both untouched in relative mode. */
    if (relative_mode) return;
    if (!absolute) return;
    unsigned type = (*event)[0];
    uint16_t *xy = (uint16_t *)(void *)(*event + 4);
    if (type == 4) {
        int16_t *rel = (int16_t *)(void *)(*event + 8);
        uint16_t x = rel[0] < 0 ? 0 : rel[0] > 639 ? 639 : (uint16_t)rel[0];
        uint16_t y = rel[1] < 0 ? 0 : rel[1] > 479 ? 479 : (uint16_t)rel[1];
        xy[0] = x;
        xy[1] = y;
        last_x = x;
        last_y = y;
        have_position = 1;
    } else if (absolute && (type == 5 || type == 6) && have_position) {
        xy[0] = last_x;
        xy[1] = last_y;
    }
    if (records < 160 && (type == 4 || type == 5 || type == 6))
        fprintf(stderr, "P2-SDL normalized type=%u xy=%u,%u rel=%d,%d\n", type, xy[0], xy[1],
                type == 4 ? ((int16_t *)(void *)(*event + 8))[0] : 0,
                type == 4 ? ((int16_t *)(void *)(*event + 8))[1] : 0);
}
static void record(const char *api, const SDL_Event *event) {
    if (!event || records >= 160) return;
    unsigned t = (*event)[0];
    if (t < 2 || t > 6) return;
    ++records;
    if (t == 4) {
        const int16_t *v = (const int16_t *)(const void *)(*event + 4);
        fprintf(stderr, "P2-SDL %s type=%u xy=%d,%d rel=%d,%d\n", api, t, v[0], v[1], v[2], v[3]);
    } else if (t == 5 || t == 6) {
        const uint16_t *xy = (const uint16_t *)(const void *)(*event + 4);
        fprintf(stderr, "P2-SDL %s type=%u button=%u xy=%u,%u\n", api, t, (*event)[2], xy[0], xy[1]);
    } else {
        fprintf(stderr, "P2-SDL %s type=%u state=%u key=%u\n", api, t, (*event)[2], (*event)[4]);
    }
}

int SDL_GetMouseState(int *x, int *y) {
    static int (*real)(int *, int *);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_GetMouseState");
    if (!real) return 0;
    int result = real(x, y);
    if (mouse_state_records < 80) {
        ++mouse_state_records;
        fprintf(stderr, "P2-SDL GetMouseState x=%d y=%d buttons=%d\n",
                x ? *x : -1, y ? *y : -1, result);
    }
    return result;
}

int SDL_GetRelativeMouseState(int *x, int *y) {
    static int (*real)(int *, int *);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_GetRelativeMouseState");
    if (!real) return 0;
    int result = real(x, y);
    if (mouse_state_records < 80) {
        ++mouse_state_records;
        fprintf(stderr, "P2-SDL GetRelativeMouseState x=%d y=%d buttons=%d\n",
                x ? *x : -1, y ? *y : -1, result);
    }
    return result;
}

int SDL_ShowCursor(int toggle) {
    static int (*real)(int);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_ShowCursor");
    if (!real) return -1;
    int effective = force_cursor_toggle(toggle);
    int result = real(effective);
    if (cursor_records < 80) {
        ++cursor_records;
        fprintf(stderr, "P2-SDL ShowCursor requested=%d effective=%d result=%d\n",
                toggle, effective, result);
    }
    return result;
}

int SDL_WM_GrabInput(int mode) {
    static int (*real)(int);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_WM_GrabInput");
    if (!real) return -1;
    int effective = force_grab_mode(mode);
    int result = real(effective);
    if (cursor_records < 80) {
        ++cursor_records;
        fprintf(stderr, "P2-SDL WM_GrabInput requested=%d effective=%d result=%d\n",
                mode, effective, result);
    }
    return result;
}

int SDL_PollEvent(SDL_Event *event) {
    static int (*real)(SDL_Event *);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_PollEvent");
    if (!real) return 0;
    unsigned before = peep_gets;
    int result = real(event);
    if (result == 1) {
        record("Poll", event);
        /* SDL_PollEvent usually calls SDL_PeepEvents internally; applying
         * relative deltas twice would erase the movement. */
        if (peep_gets == before) {
            normalize(event);
            publish_cursor(event);
        }
    }
    return result;
}

int SDL_PeepEvents(SDL_Event *events, int count, int action, uint32_t mask) {
    static int (*real)(SDL_Event *, int, int, uint32_t);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_PeepEvents");
    if (!real) return -1;
    int result = real(events, count, action, mask);
    if (result > 0 && events && action == 2) {
        ++peep_gets;
        for (int i = 0; i < result && i < 8; ++i) {
            record("Peep", events + i);
            normalize(events + i);
            publish_cursor(events + i);
        }
    }
    return result;
}
