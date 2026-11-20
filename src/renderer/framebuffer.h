#ifndef CORTEX_FRAMEBUFFER_H
#define CORTEX_FRAMEBUFFER_H

#include "pixel_draw.h"

typedef struct Framebuffer Framebuffer;

Framebuffer *fb_create(void);
void         fb_destroy(Framebuffer *fb);

/* Upload canvas → GPU e desenha o quad fullscreen. */
void         fb_present(Framebuffer *fb, const Canvas pixels);

#endif
