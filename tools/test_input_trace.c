/* Host-side deterministic test for the guest mouse coordinate classifier. */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/postal2_sdl_input_trace.c"

static void accumulate_mouse_delta(int *x, int *y, int dx, int dy);
static int mock_cursor_x, mock_cursor_y, mock_cursor_on, mock_cursor_calls;
void postal2_fb_set_cursor(int x, int y, int on) {
    mock_cursor_x = x;
    mock_cursor_y = y;
    mock_cursor_on = on;
    ++mock_cursor_calls;
}

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
    int mouse_x = 0, mouse_y = 0;
    accumulate_mouse_delta(&mouse_x, &mouse_y, 256, 192);
    assert(mouse_x == 256 && mouse_y == 192);
    accumulate_mouse_delta(&mouse_x, &mouse_y, 2, 0);
    assert(mouse_x == 258 && mouse_y == 192);
    accumulate_mouse_delta(&mouse_x, &mouse_y, 381, 287);
    assert(mouse_x == 639 && mouse_y == 479);
    accumulate_mouse_delta(&mouse_x, &mouse_y, -1000, -1000);
    assert(mouse_x == 0 && mouse_y == 0);
    assert(setenv("POSTAL2_DIAG_UWINDOW_CURSOR", "1", 1) == 0);
    SDL_Event motion = {0};
    motion[0] = 4;
    uint16_t *motion_xy = (uint16_t *)(void *)(motion + 4);
    int16_t *motion_rel = (int16_t *)(void *)(motion + 8);
    motion_xy[0] = 639; motion_xy[1] = 479;
    motion_rel[0] = 256; motion_rel[1] = 192;
    trace_relative_projection(&motion);
    assert(mock_cursor_calls == 1 && mock_cursor_on == 1);
    assert(mock_cursor_x == 256 && mock_cursor_y == 192);
    motion_rel[0] = 383; motion_rel[1] = 287;
    trace_relative_projection(&motion);
    assert(mock_cursor_calls == 2);
    assert(mock_cursor_x == 639 && mock_cursor_y == 479);
    puts("PASS: SDL preservation/overrides and 640x480 relative-cursor projection bounds");
    return 0;
}
