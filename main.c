#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>
#include <rsx/rsx.h>

#define MAX_BUFFERS 2

typedef struct {
    int height;
    int width;
    int id;
    uint32_t *ptr;
    uint32_t offset;
} buffer;

gcmContextData *context = NULL;
videoResolution res;
buffer buffers[MAX_BUFFERS];
int currentBuffer = 0;
static int running = 1;

static void sysutil_callback(u64 status, u64 param, void *userdata) {
    if (status == SYSUTIL_EXIT_GAME) {
        running = 0;
    }
}

void waitFlip() {
    while (gcmGetFlipStatus() != 0)
        usleep(200);
    gcmResetFlipStatus();
}

void flip(s32 bufferId) {
    gcmSetFlip(context, bufferId);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);
}

void makeBuffer(buffer *buf, u16 width, u16 height, int id) {
    int size = width * height * sizeof(uint32_t);
    buf->ptr = (uint32_t *)rsxMemalign(64, size);
    rsxAddressToOffset(buf->ptr, &buf->offset);
    gcmSetDisplayBuffer(id, buf->offset, width * 4, width, height);
    buf->width = width;
    buf->height = height;
    buf->id = id;
}

void init_screen() {
    void *host_addr = memalign(1024 * 1024, 1024 * 1024);
    rsxInit(&context, 0x10000, 1024 * 1024, host_addr);

    videoState state;
    videoGetState(0, 0, &state);
    videoGetResolution(state.displayMode.resolution, &res);

    videoConfiguration vconfig;
    memset(&vconfig, 0, sizeof(videoConfiguration));
    vconfig.resolution = state.displayMode.resolution;
    vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
    vconfig.pitch = res.width * sizeof(uint32_t);

    videoConfigure(0, &vconfig, NULL, 0);
    videoGetState(0, 0, &state);

    gcmSetFlipMode(GCM_FLIP_VSYNC);
    makeBuffer(&buffers[0], res.width, res.height, 0);
    makeBuffer(&buffers[1], res.width, res.height, 1);

    gcmResetFlipStatus();
    flip(1);
}

void draw_circle(buffer *buf, int cx, int cy, int r, uint32_t color) {
    for (int y = cy - r; y <= cy + r; y++) {
        if (y < 0 || y >= buf->height) continue;
        for (int x = cx - r; x <= cx + r; x++) {
            if (x < 0 || x >= buf->width) continue;
            int dx = x - cx;
            int dy = y - cy;
            if (dx * dx + dy * dy <= r * r) {
                buf->ptr[y * buf->width + x] = color;
            }
        }
    }
}

void draw_pou(buffer *buf, int cx, int cy, int eating) {
    uint32_t body_color = 0x00B8824A;
    uint32_t dark_edge  = 0x007A5229;
    uint32_t white      = 0x00FFFFFF;
    uint32_t black      = 0x00111111;
    uint32_t mouth_red  = 0x00D94A38;

    draw_circle(buf, cx, cy + 25, 115, dark_edge);
    draw_circle(buf, cx, cy + 25, 108, body_color);
    draw_circle(buf, cx, cy - 30, 88, dark_edge);
    draw_circle(buf, cx, cy - 30, 81, body_color);

    draw_circle(buf, cx - 32, cy - 45, 24, black);
    draw_circle(buf, cx - 32, cy - 45, 21, white);
    draw_circle(buf, cx - 28, cy - 45, 10, black);

    draw_circle(buf, cx + 32, cy - 45, 24, black);
    draw_circle(buf, cx + 32, cy - 45, 21, white);
    draw_circle(buf, cx + 28, cy - 45, 10, black);

    if (eating) {
        draw_circle(buf, cx, cy + 15, 26, black);
        draw_circle(buf, cx, cy + 18, 20, mouth_red);
    } else {
        draw_circle(buf, cx, cy + 12, 16, black);
        draw_circle(buf, cx, cy + 6, 15, body_color);
    }
}

void draw_apple(buffer *buf, int cx, int cy) {
    draw_circle(buf, cx, cy, 24, 0x00E02424);
    draw_circle(buf, cx + 6, cy - 26, 8, 0x0032CD32);
}

int main(int argc, char *argv[]) {
    init_screen();
    ioPadInit(7);
    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    padInfo padinfo;
    padData paddata;
    int eat_timer = 0;
    int apple_x = 0, apple_y = 0;

    while (running) {
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);

        if (padinfo.status) {
            ioPadGetData(0, &paddata);
            if (paddata.BTN_CROSS && eat_timer == 0) {
                eat_timer = 35;
                apple_x = (res.width / 2) + 180;
                apple_y = (res.height / 2) + 20;
            }
            if (paddata.BTN_START) {
                running = 0;
            }
        }

        waitFlip();
        buffer *buf = &buffers[currentBuffer];

        uint32_t bg_color = 0x0087CEEB;
        int total_pixels = buf->width * buf->height;
        for (int i = 0; i < total_pixels; i++) {
            buf->ptr[i] = bg_color;
        }

        int cx = res.width / 2;
        int cy = res.height / 2;

        if (eat_timer > 0) {
            draw_pou(buf, cx, cy - 15, 1);
            apple_x -= 5;
            draw_apple(buf, apple_x, apple_y);
            eat_timer--;
        } else {
            draw_pou(buf, cx, cy, 0);
        }

        flip(currentBuffer);
        currentBuffer = !currentBuffer;
    }

    return 0;
}
