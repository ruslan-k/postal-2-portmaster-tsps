/* Host-side deterministic test for the guest mouse coordinate classifier. */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/postal2_sdl_input_trace.c"

int main(void) {
    SDL_Event event = {0};
    uint16_t *xy = (uint16_t *)(void *)(event + 4);
    int16_t *rel = (int16_t *)(void *)(event + 8);
    event[0] = 4;
    xy[0] = 639; xy[1] = 479;
    rel[0] = 320; rel[1] = 245;
    assert(setenv("POSTAL2_MOUSE_COORD_MODE", "passive", 1) == 0);
    normalize(&event);
    assert(xy[0] == 639 && xy[1] == 479);
    assert(setenv("POSTAL2_MOUSE_COORD_MODE", "absolute", 1) == 0);
    normalize(&event);
    assert(xy[0] == 320 && xy[1] == 245);
    assert(rel[0] == 320 && rel[1] == 245);
    event[0] = 5;
    xy[0] = 639; xy[1] = 479;
    normalize(&event);
    assert(xy[0] == 320 && xy[1] == 245);
    event[0] = 6;
    xy[0] = 639; xy[1] = 479;
    normalize(&event);
    assert(xy[0] == 320 && xy[1] == 245);
    event[0] = 2;
    event[4] = 39;
    normalize(&event);
    assert(event[4] == 39);
    assert(setenv("POSTAL2_MOUSE_COORD_MODE", "relative", 1) == 0);
    event[0] = 4;
    xy[0] = 256; xy[1] = 192;
    rel[0] = 256; rel[1] = 192;
    normalize(&event);
    assert(xy[0] == 256 && xy[1] == 192);
    assert(rel[0] == 256 && rel[1] == 192);
    int sum_x = rel[0], sum_y = rel[1];
    xy[0] = 258; xy[1] = 192;
    rel[0] = 2; rel[1] = 0;
    normalize(&event);
    sum_x += rel[0]; sum_y += rel[1];
    assert(xy[0] == 258 && xy[1] == 192);
    assert(rel[0] == 2 && rel[1] == 0);
    xy[0] = 639; xy[1] = 479;
    rel[0] = 381; rel[1] = 287;
    normalize(&event);
    sum_x += rel[0]; sum_y += rel[1];
    assert(sum_x == 639 && sum_y == 479);
    assert(xy[0] == 639 && xy[1] == 479);
    event[0] = 5;
    xy[0] = 639; xy[1] = 479;
    normalize(&event);
    assert(xy[0] == 639 && xy[1] == 479);
    puts("PASS: SDL relative events preserved byte-for-byte; full-range native delta sequence preserved");
    return 0;
}
