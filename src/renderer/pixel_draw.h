#ifndef CORTEX_PIXEL_DRAW_H
#define CORTEX_PIXEL_DRAW_H

#include <stdint.h>
#include "config.h"

/*
 * Pixel armazenado como RGBA8 little-endian:
 *   bytes em memória → [R, G, B, A]
 *   uint32_t         → 0xAABBGGRR
 * Compatível com glTexImage2D(..., GL_RGBA, GL_UNSIGNED_BYTE, ...).
 */
typedef uint32_t Pixel;
typedef Pixel Canvas[CANVAS_W * CANVAS_H];

#define PX(r,g,b,a) \
    ((uint32_t)(r) | ((uint32_t)(g)<<8) | ((uint32_t)(b)<<16) | ((uint32_t)(a)<<24))

/* paleta */
#define COL_TRANSPARENT PX(  0,  0,  0,  0)
#define COL_WHITE       PX(255,255,255,255)
#define COL_BORDER      PX(217,221,226,255)
#define COL_IRIS_DARK   PX(  7, 85, 32,255)
#define COL_IRIS_MID    PX(  7,132, 47,255)
#define COL_IRIS_BRIGHT PX( 22,201, 67,255)
#define COL_PUPIL       PX( 16, 20, 16,255)
#define COL_MOUTH       PX( 23, 23, 23,255)

void pd_clear       (Canvas buf, Pixel c);
void pd_set         (Canvas buf, int x, int y, Pixel c);
void pd_fill_rect   (Canvas buf, int x, int y, int w, int h, Pixel c);
void pd_fill_circle (Canvas buf, int cx, int cy, int r, Pixel c);
void pd_fill_ellipse(Canvas buf, int cx, int cy, int rx, int ry, Pixel c);
void pd_rounded_rect(Canvas buf, int x, int y, int w, int h, int r,
                     Pixel fill, Pixel border);
/* arco parabólico: peak>0 = sorriso, peak<0 = tristeza */
void pd_parabola_arc(Canvas buf, int cx, int cy, int hw,
                     int peak, int thickness, Pixel c);

#endif
