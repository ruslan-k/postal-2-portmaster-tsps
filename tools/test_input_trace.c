/* Host-side deterministic test for SDL tracing and the shared cursor frame. */
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../src/postal2_frame.h"
#include "../src/postal2_sdl_input_trace.c"

static volatile uint32_t *map_cursor_frame(const char *path);
static int store_cursor_frame_header(volatile uint32_t *hdr, int x, int y, int on);
static int publish_cursor_event(volatile uint32_t *hdr, SDL_Event *event, int *x_out, int *y_out);

static int cursor_event_position(const SDL_Event *event, int *x, int *y);
static void publish_cursor(SDL_Event *event);

static void test_cursor_frame_header_mapping(void) {
    char path[] = "/tmp/postal2-frame-test-XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    assert(ftruncate(fd, POSTAL2_FRAME_HDR) == 0);
    uint32_t initial[POSTAL2_FRAME_HDR / 4] = {0};
    initial[POSTAL2_HDR_MAGIC] = POSTAL2_FRAME_MAGIC;
    assert(pwrite(fd, initial, sizeof(initial), 0) == (ssize_t)sizeof(initial));
    assert(close(fd) == 0);

    volatile uint32_t *hdr = map_cursor_frame(path);
    assert(hdr != NULL);
    SDL_Event motion = {0};
    uint16_t *motion_xy = (uint16_t *)(void *)(motion + 4);
    int16_t *motion_rel = (int16_t *)(void *)(motion + 8);
    motion[0] = 4;
    motion_xy[0] = 320; motion_xy[1] = 240;
    motion_rel[0] = 3; motion_rel[1] = -2;
    int marker_x = -1, marker_y = -1;
    assert(publish_cursor_event(hdr, &motion, &marker_x, &marker_y) == 1);
    assert(marker_x == 320 && marker_y == 240);
    assert(hdr[POSTAL2_HDR_CURSOR_X] == 320);
    assert(hdr[POSTAL2_HDR_CURSOR_Y] == 240);
    assert(hdr[POSTAL2_HDR_CURSOR_ON] == 1);
    assert(motion_xy[0] == 320 && motion_xy[1] == 240);
    assert(motion_rel[0] == 3 && motion_rel[1] == -2);
    SDL_Event down = {0};
    uint16_t *down_xy = (uint16_t *)(void *)(down + 4);
    down[0] = 5;
    down[8] = 0x7f; down[10] = 0x6f;
    down_xy[0] = 639; down_xy[1] = 479;
    assert(publish_cursor_event(hdr, &down, &marker_x, &marker_y) == 1);
    assert(marker_x == 639 && marker_y == 479);
    assert(hdr[POSTAL2_HDR_CURSOR_X] == 639);
    assert(hdr[POSTAL2_HDR_CURSOR_Y] == 479);
    SDL_Event up = {0};
    uint16_t *up_xy = (uint16_t *)(void *)(up + 4);
    up[0] = 6;
    up_xy[0] = 639; up_xy[1] = 479;
    assert(publish_cursor_event(hdr, &up, &marker_x, &marker_y) == 1);
    assert(marker_x == 639 && marker_y == 479);
    SDL_Event key = {0};
    key[0] = 2;
    assert(publish_cursor_event(hdr, &key, &marker_x, &marker_y) == 0);
    assert(hdr[POSTAL2_HDR_CURSOR_X] == 639);
    assert(hdr[POSTAL2_HDR_CURSOR_Y] == 479);
    assert(store_cursor_frame_header(hdr, POSTAL2_FRAME_MAX_W, 0, 1) == 0);
    assert(store_cursor_frame_header(hdr, 0, POSTAL2_FRAME_MAX_H, 1) == 0);
    assert(store_cursor_frame_header(hdr, -1, 0, 1) == 0);
    assert(hdr[POSTAL2_HDR_CURSOR_X] == 639);
    assert(hdr[POSTAL2_HDR_CURSOR_Y] == 479);
    assert(munmap((void *)hdr, POSTAL2_FRAME_HDR) == 0);

    fd = open(path, O_RDONLY);
    assert(fd >= 0);
    uint32_t readback[POSTAL2_FRAME_HDR / 4] = {0};
    assert(pread(fd, readback, sizeof(readback), 0) == (ssize_t)sizeof(readback));
    assert(readback[POSTAL2_HDR_CURSOR_X] == 639);
    assert(readback[POSTAL2_HDR_CURSOR_Y] == 479);
    assert(readback[POSTAL2_HDR_CURSOR_ON] == 1);
    assert(close(fd) == 0);
    fd = open(path, O_RDWR);
    assert(fd >= 0);
    uint32_t bad_magic = 0;
    assert(pwrite(fd, &bad_magic, sizeof(bad_magic), 0) == (ssize_t)sizeof(bad_magic));
    assert(close(fd) == 0);
    assert(map_cursor_frame(path) == NULL);
    assert(unlink(path) == 0);
}

int main(void) {
    test_cursor_frame_header_mapping();
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
    assert(setenv("POSTAL2_FORCE_CURSOR", "0", 1) == 0);
    assert(force_cursor_toggle(0) == 0);
    assert(force_cursor_toggle(1) == 1);
    assert(force_cursor_toggle(-1) == -1);
    assert(setenv("POSTAL2_FORCE_CURSOR", "1", 1) == 0);
    assert(force_cursor_toggle(0) == 1);
    assert(force_cursor_toggle(1) == 1);
    assert(force_cursor_toggle(-1) == -1);
    assert(setenv("POSTAL2_FORCE_UNGRAB", "0", 1) == 0);
    assert(force_grab_mode(-1) == -1);
    assert(force_grab_mode(0) == 0);
    assert(force_grab_mode(1) == 1);
    assert(setenv("POSTAL2_FORCE_UNGRAB", "1", 1) == 0);
    assert(force_grab_mode(-1) == -1);
    assert(force_grab_mode(0) == 0);
    assert(force_grab_mode(1) == 0);
    assert(setenv("POSTAL2_DIAG_UWINDOW_CURSOR", "0", 1) == 0);
    publish_cursor(&event);
    puts("PASS: raw SDL x/y are written to the shared presenter frame without mutating the event");
    return 0;
}
