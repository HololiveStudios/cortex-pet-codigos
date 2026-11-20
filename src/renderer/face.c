#include "face.h"
#include <math.h>
#include <string.h>

/* ── constantes de layout no canvas 128×128 ── */
#define BODY_X  10
#define BODY_Y   8
#define BODY_W 108
#define BODY_H 112
#define BODY_R   9

#define EYE_L_CX 40
#define EYE_R_CX 88
#define EYE_CY   50
#define EYE_RX   11   /* raio horizontal iris */
#define EYE_RY   11   /* raio vertical iris (openness=1) */
#define PUPIL_R   5

#define MOUTH_CX 64
#define MOUTH_CY 90

static void draw_eye(Canvas buf, int cx, int cy,
                     const EyeState *e, float blink_lid) {
    float open = e->openness < 0.f ? 0.f : (e->openness > 1.3f ? 1.3f : e->openness);
    int ry = (int)roundf((float)EYE_RY * open);

    if (ry == 0) {
        /* fechado: linha horizontal */
        for (int dx = -EYE_RX; dx <= EYE_RX; dx++) {
            pd_set(buf, cx+dx, cy,   COL_PUPIL);
            pd_set(buf, cx+dx, cy-1, COL_PUPIL);
        }
        return;
    }

    /* ── iris em camadas ── */
    pd_fill_ellipse(buf, cx, cy, EYE_RX, ry,
                    COL_IRIS_DARK);
    if (ry > 2)
        pd_fill_ellipse(buf, cx, cy, EYE_RX-2, ry-2,
                        COL_IRIS_MID);
    if (ry > 4)
        pd_fill_ellipse(buf, cx, cy, EYE_RX-4, ry-4,
                        COL_IRIS_BRIGHT);

    /* ── pupila ── */
    float mpx = (float)(EYE_RX - PUPIL_R - 1) * 0.65f;
    float mpy = (float)(ry     - PUPIL_R - 1) * 0.65f;
    if (mpx < 0) mpx = 0; if (mpy < 0) mpy = 0;
    int pcx = cx + (int)roundf(e->pupil_x * mpx);
    int pcy = cy + (int)roundf(e->pupil_y * mpy);
    pd_fill_circle(buf, pcx, pcy, PUPIL_R, COL_PUPIL);

    /* ── reflexos ── */
    pd_fill_rect(buf, pcx-4, pcy-4, 3, 2, COL_WHITE);
    pd_set(buf,       pcx+1, pcy-3,       COL_WHITE);

    /* ── pálpebra (expressão + piscar) ── */
    float lid = e->expr_lid > blink_lid ? e->expr_lid : blink_lid;
    if (lid > 0.01f) {
        int lid_h = (int)ceilf((float)(ry + 2) * lid) + 1;
        int top   = cy - ry - 1;
        /* largura generosa para cobrir bordas da íris corretamente */
        pd_fill_rect(buf, cx - EYE_RX - 2, top,
                     2*(EYE_RX+2)+4, lid_h, COL_WHITE);
    }
}

static void draw_mouth(Canvas buf, int cx, int cy, const MouthState *m) {
    int hw = (int)roundf(8.f + m->width * 8.f);   /* 8..16 half-width */
    if (hw < 2) hw = 2;

    switch (m->shape) {
    case MOUTH_NEUTRAL:
        pd_fill_rect(buf, cx-hw, cy, hw*2, 2, COL_MOUTH);
        break;

    case MOUTH_SMILE: {
        int peak = (int)roundf(m->curvature * 7.f);
        pd_parabola_arc(buf, cx, cy, hw, peak, 2, COL_MOUTH);
        break;
    }

    case MOUTH_FROWN: {
        int peak = -(int)roundf(m->curvature * 7.f);
        pd_parabola_arc(buf, cx, cy, hw, peak, 2, COL_MOUTH);
        break;
    }

    case MOUTH_OPEN: {
        int ew = hw;
        int eh = (int)roundf(m->openness * 10.f);
        if (eh < 1) eh = 1;
        pd_fill_ellipse(buf, cx, cy + eh/2, ew, eh, COL_MOUTH);
        break;
    }

    case MOUTH_WORRIED:
        /* linha curta levemente assimétrica */
        pd_fill_rect(buf, cx-5, cy, 10, 2, COL_MOUTH);
        pd_set(buf, cx-6, cy+1,          COL_MOUTH);
        pd_set(buf, cx+5, cy-1,          COL_MOUTH);
        break;
    }
}

/* ── API pública ── */

void face_default(FaceState *f) {
    memset(f, 0, sizeof(*f));
    f->left_eye.openness  = 1.f;
    f->right_eye.openness = 1.f;
    f->mouth.shape        = MOUTH_NEUTRAL;
    f->mouth.width        = 0.4f;
    f->mouth.curvature    = 0.5f;
}

void face_draw(Canvas buf, const FaceState *face, int bob_y, float blink_lid) {
    pd_clear(buf, COL_TRANSPARENT);

    /* corpo */
    pd_rounded_rect(buf,
                    BODY_X, BODY_Y + bob_y,
                    BODY_W, BODY_H, BODY_R,
                    COL_WHITE, COL_BORDER);

    /* olhos */
    draw_eye(buf, EYE_L_CX, EYE_CY + bob_y, &face->left_eye,  blink_lid);
    draw_eye(buf, EYE_R_CX, EYE_CY + bob_y, &face->right_eye, blink_lid);

    /* boca */
    draw_mouth(buf, MOUTH_CX, MOUTH_CY + bob_y, &face->mouth);
}
