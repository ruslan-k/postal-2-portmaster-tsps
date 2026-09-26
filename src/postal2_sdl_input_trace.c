/* i386 SDL 1.2 mouse classifier: passive, absolute, or relative A/B. */
/* No 32-bit development headers are required to cross-build this probe. */
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef struct _IO_FILE FILE;
extern FILE *stderr;
extern int fprintf(FILE *, const char *, ...);
extern void *dlsym(void *, const char *);
extern char *getenv(const char *);
#define RTLD_NEXT ((void *)-1)

typedef unsigned char SDL_Event[24];
static unsigned records;
static uint16_t last_x, last_y;
static int have_position;
static unsigned peep_gets;
static int mode_is(const char *wanted) {
    const char *mode = getenv("POSTAL2_MOUSE_COORD_MODE");
    if (!mode) return 0;
    while (*wanted && *mode && *wanted == *mode) { ++wanted; ++mode; }
    return !*wanted && !*mode;
}
static void normalize(SDL_Event *event) {
    if (!event) return;
    int absolute = mode_is("absolute");
    int relative_mode = mode_is("relative");
    if (!absolute && !relative_mode) return;
    unsigned type = (*event)[0];
    uint16_t *xy = (uint16_t *)(void *)(*event + 4);
    if (type == 4) {
        int16_t *rel = (int16_t *)(void *)(*event + 8);
        /* The raw xrel/yrel track pointer position, not motion deltas.
         * Compare one field family at a time: absolute leaves rel alone;
         * relative replaces rel with consecutive-position differences. */
        uint16_t x = rel[0] < 0 ? 0 : rel[0] > 639 ? 639 : (uint16_t)rel[0];
        uint16_t y = rel[1] < 0 ? 0 : rel[1] > 479 ? 479 : (uint16_t)rel[1];
        if (relative_mode) {
            /* Seed the menu's logical cursor from the first real pointer
             * position. Starting at zero loses the initial 256x192 offset,
             * so the cursor can traverse only the upper-left subset. */
            rel[0] = have_position ? (int16_t)(x - last_x) : (int16_t)x;
            rel[1] = have_position ? (int16_t)(y - last_y) : (int16_t)y;
        } else {
            xy[0] = x;
            xy[1] = y;
        }
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
        if (peep_gets == before) normalize(event);
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
        }
    }
    return result;
}
