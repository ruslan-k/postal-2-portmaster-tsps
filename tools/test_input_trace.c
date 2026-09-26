/* Host-side deterministic test for the guest mouse coordinate classifier. */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/postal2_sdl_input_trace.c"
#define RTLD_LAZY 1
#define RTLD_NOLOAD 4

static int cursor_event_position(const SDL_Event *event, int *x, int *y);
static void publish_cursor(SDL_Event *event);
static int mock_cursor_x, mock_cursor_y, mock_cursor_on, mock_cursor_calls;
static void *mock_egl_handle = (void *)1;
static int mock_default_resolves = 1;
static int mock_egl_loaded;
static int mock_handle_resolves = 1;
static unsigned mock_default_lookups, mock_handle_lookups, mock_dlopen_calls;
void postal2_fb_set_cursor(int x, int y, int on) {
    mock_cursor_x = x;
    mock_cursor_y = y;
    mock_cursor_on = on;
    ++mock_cursor_calls;
}
void *dlsym(void *handle, const char *name) {
    if (strcmp(name, "postal2_fb_set_cursor") != 0) return NULL;
    if (handle == RTLD_DEFAULT) {
        ++mock_default_lookups;
        return mock_default_resolves ? (void *)postal2_fb_set_cursor : NULL;
    }
    if (handle == mock_egl_handle) {
        ++mock_handle_lookups;
        return mock_handle_resolves ? (void *)postal2_fb_set_cursor : NULL;
    }
    return NULL;
}
void *dlopen(const char *name, int flags) {
    assert(strcmp(name, "libEGL.so.1") == 0);
    assert(flags == (RTLD_LAZY | RTLD_NOLOAD));
    ++mock_dlopen_calls;
    return mock_egl_loaded ? mock_egl_handle : NULL;
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
    assert(setenv("POSTAL2_DIAG_UWINDOW_CURSOR", "1", 1) == 0);
    SDL_Event motion = {0};
    uint16_t *motion_xy = (uint16_t *)(void *)(motion + 4);
    int16_t *motion_rel = (int16_t *)(void *)(motion + 8);
    motion[0] = 4;
    motion_xy[0] = 320; motion_xy[1] = 245;
    motion_rel[0] = 2; motion_rel[1] = -3;
    int cursor_x = -1, cursor_y = -1;
    assert(cursor_event_position(&motion, &cursor_x, &cursor_y) == 1);
    assert(cursor_x == 320 && cursor_y == 245);
    publish_cursor(&motion);
    assert(mock_cursor_calls == 1 && mock_cursor_on == 1);
    assert(mock_cursor_x == 320 && mock_cursor_y == 245);
    assert(motion_rel[0] == 2 && motion_rel[1] == -3);
    SDL_Event down = {0};
    uint16_t *button_xy = (uint16_t *)(void *)(down + 4);
    down[0] = 5;
    down[8] = 0x7f; down[10] = 0x6f;
    button_xy[0] = 77; button_xy[1] = 88;
    assert(cursor_event_position(&down, &cursor_x, &cursor_y) == 1);
    assert(cursor_x == 77 && cursor_y == 88);
    publish_cursor(&down);
    assert(mock_cursor_calls == 2 && mock_cursor_x == 77 && mock_cursor_y == 88);
    SDL_Event up = {0};
    up[0] = 6;
    uint16_t *up_xy = (uint16_t *)(void *)(up + 4);
    up_xy[0] = 91; up_xy[1] = 92;
    publish_cursor(&up);
    assert(mock_cursor_calls == 3 && mock_cursor_x == 91 && mock_cursor_y == 92);
    diag_cursor_lookup_done = 0;
    diag_cursor_set = NULL;
    mock_default_resolves = 0;
    mock_egl_loaded = 1;
    publish_cursor(&motion);
    assert(mock_cursor_calls == 4 && mock_cursor_x == 320 && mock_cursor_y == 245);
    assert(mock_default_lookups == 2);
    assert(mock_dlopen_calls == 1 && mock_handle_lookups == 1);
    SDL_Event other = {0};
    other[0] = 2;
    assert(cursor_event_position(&other, &cursor_x, &cursor_y) == 0);
    assert(setenv("POSTAL2_DIAG_UWINDOW_CURSOR", "0", 1) == 0);
    publish_cursor(&motion);
    assert(mock_cursor_calls == 4);
    puts("PASS: presenter reference uses raw SDL x/y for motion and button events");
    return 0;
}
