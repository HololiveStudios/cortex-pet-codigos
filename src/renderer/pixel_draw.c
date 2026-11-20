#include "pixel_draw.h"
#include <math.h>
#include <string.h>

void pd_clear(Canvas buf, Pixel c) {
    for (int i = 0; i < CANVAS_W * CANVAS_H; i++) buf[i] = c;
}

static inline int in_bounds(int x, int y) {
    return (unsigned)x < (unsigned)CANVAS_W &&
           (unsigned)y < (unsigned)CANVAS_H;
}

void pd_set(Canvas buf, int x, int y, Pixel c) {
    if (in_bounds(x, y)) buf[y * CANVAS_W + x] = c;
}

void pd_fill_rect(Canvas buf, int x, int y, int w, int h, Pixel c) {
    for (int r = y; r < y + h; r++)
        for (int col = x; col < x + w; col++)
            pd_set(buf, col, r, c);
}

void pd_fill_circle(Canvas buf, int cx, int cy, int r, Pixel c) {
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx*dx + dy*dy <= r*r)
                pd_set(buf, cx+dx, cy+dy, c);
}

void pd_fill_ellipse(Canvas buf, int cx, int cy, int rx, int ry, Pixel c) {
    if (rx <= 0 || ry <= 0) return;
    float irx2 = 1.f / (float)(rx * rx);
    float iry2 = 1.f / (float)(ry * ry);
    for (int dy = -ry; dy <= ry; dy++)
        for (int dx = -rx; dx <= rx; dx++)
            if ((float)(dx*dx)*irx2 + (float)(dy*dy)*iry2 <= 1.f)
                pd_set(buf, cx+dx, cy+dy, c);
}

static int inside_rrect(int col, int row,
                         int x0, int y0, int x1, int y1, int r) {
    if (col < x0 || col > x1 || row < y0 || row > y1) return 0;
    /* checar cantos */
    int cl = col < x0+r, cr = col > x1-r;
    int ct = row < y0+r, cb = row > y1-r;
    if (cl && ct) { int dx=col-(x0+r), dy=row-(y0+r); return dx*dx+dy*dy<=r*r; }
    if (cr && ct) { int dx=col-(x1-r), dy=row-(y0+r); return dx*dx+dy*dy<=r*r; }
    if (cl && cb) { int dx=col-(x0+r), dy=row-(y1-r); return dx*dx+dy*dy<=r*r; }
    if (cr && cb) { int dx=col-(x1-r), dy=row-(y1-r); return dx*dx+dy*dy<=r*r; }
    return 1;
}

void pd_rounded_rect(Canvas buf, int x, int y, int w, int h, int r,
                     Pixel fill, Pixel border) {
    int x1 = x+w-1, y1 = y+h-1;
    for (int row = y; row <= y1; row++) {
        for (int col = x; col <= x1; col++) {
            if (!inside_rrect(col, row, x, y, x1, y1, r)) continue;
            /* borda: algum vizinho fora do rrect */
            int is_border =
                !inside_rrect(col-1, row,   x, y, x1, y1, r) ||
                !inside_rrect(col+1, row,   x, y, x1, y1, r) ||
                !inside_rrect(col,   row-1, x, y, x1, y1, r) ||
                !inside_rrect(col,   row+1, x, y, x1, y1, r);
            pd_set(buf, col, row, is_border ? border : fill);
        }
    }
}

void pd_parabola_arc(Canvas buf, int cx, int cy, int hw,
                     int peak, int thickness, Pixel c) {
    if (hw <= 0) return;
    for (int dx = -hw; dx <= hw; dx++) {
        float t  = (float)dx / (float)hw;
        float yf = (float)cy + (float)peak * (1.f - t*t);
        int   yi = (int)roundf(yf);
        for (int th = 0; th < thickness; th++)
            pd_set(buf, cx+dx, yi+th, c);
    }
}
