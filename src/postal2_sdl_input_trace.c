/* Passive i386 SDL 1.2 input probe. Never changes an event or SDL result. */
/* No 32-bit development headers are required to cross-build this probe. */
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef struct _IO_FILE FILE;
extern FILE *stderr;
extern int fprintf(FILE *, const char *, ...);
extern void *dlsym(void *, const char *);
#define RTLD_NEXT ((void *)-1)

typedef unsigned char SDL_Event[24];
static unsigned records;
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
    int result = real(event);
    if (result == 1) record("Poll", event);
    return result;
}

int SDL_PeepEvents(SDL_Event *events, int count, int action, uint32_t mask) {
    static int (*real)(SDL_Event *, int, int, uint32_t);
    if (!real) real = dlsym(RTLD_NEXT, "SDL_PeepEvents");
    if (!real) return -1;
    int result = real(events, count, action, mask);
    if (result > 0 && events && action == 2) {
        for (int i = 0; i < result && i < 8; ++i) record("Peep", events + i);
    }
    return result;
}
