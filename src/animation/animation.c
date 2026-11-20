#include "animation.h"
#include "neuro.h"   /* brain_motor() — header no root */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846

static float lf(float a, float b, float t) { return a + (b-a)*t; }

/* lerp suave baseado em dt */
static float smooth(float a, float b, double dt, float speed) {
    float t = (float)(1.0 - exp(-(double)speed * dt));
    return lf(a, b, t);
}

static void lerp_eye(EyeState *cur, const EyeState *tgt, float t) {
    cur->openness  = lf(cur->openness,  tgt->openness,  t);
    cur->pupil_x   = lf(cur->pupil_x,   tgt->pupil_x,   t);
    cur->pupil_y   = lf(cur->pupil_y,   tgt->pupil_y,   t);
    cur->expr_lid  = lf(cur->expr_lid,  tgt->expr_lid,  t);
}

static void lerp_mouth(MouthState *cur, const MouthState *tgt, float t) {
    if (cur->shape != tgt->shape && t > 0.5f) cur->shape = tgt->shape;
    cur->openness  = lf(cur->openness,  tgt->openness,  t);
    cur->width     = lf(cur->width,     tgt->width,     t);
    cur->curvature = lf(cur->curvature, tgt->curvature, t);
}

void anim_init(AnimState *a) {
    memset(a, 0, sizeof(*a));
    face_default(&a->current);
    face_default(&a->target);
    a->blink_timer = 3.0 + 2.0 * ((double)rand() / RAND_MAX);
    a->bob_phase   = 0.0;
    a->pupil_timer = 0.0;
}

void anim_sync_brain(AnimState *a,
                     const Brain *brain,
                     const Drives *drives,
                     const Emotion *emotion) {
    (void)emotion;
    float happy = (float)brain_motor(brain, 0);
    float sad   = (float)brain_motor(brain, 1);
    float sleep = (float)brain_motor(brain, 2);
    float surpr = (float)brain_motor(brain, 3);
    float angry = (float)brain_motor(brain, 4);
    float curio = (float)brain_motor(brain, 5);
    float play  = (float)brain_motor(brain, 6);

    FaceState *t = &a->target;
    face_default(t);

    if (sleep > 0.40f) {
        /* dormindo */
        t->left_eye.openness  = 0.05f;
        t->right_eye.openness = 0.05f;
        t->left_eye.expr_lid  = 0.9f;
        t->right_eye.expr_lid = 0.9f;
        t->mouth.shape        = MOUTH_NEUTRAL;
        t->mouth.width        = 0.25f;

    } else if (surpr > 0.40f) {
        /* surpreso */
        t->left_eye.openness  = 1.25f;
        t->right_eye.openness = 1.25f;
        t->mouth.shape        = MOUTH_OPEN;
        t->mouth.openness     = 0.75f;
        t->mouth.width        = 0.55f;

    } else if (angry > 0.38f) {
        /* irritado */
        t->left_eye.openness  = 0.75f;
        t->right_eye.openness = 0.75f;
        t->left_eye.expr_lid  = 0.30f;
        t->right_eye.expr_lid = 0.30f;
        t->mouth.shape        = MOUTH_WORRIED;
        t->mouth.width        = 0.30f;

    } else if (play > 0.35f || happy > 0.35f) {
        /* feliz/animado */
        float boost           = play > happy ? play : happy;
        t->left_eye.openness  = 1.0f + boost * 0.15f;
        t->right_eye.openness = t->left_eye.openness;
        t->mouth.shape        = boost > 0.55f ? MOUTH_OPEN : MOUTH_SMILE;
        t->mouth.openness     = boost;
        t->mouth.width        = 0.5f + boost * 0.5f;
        t->mouth.curvature    = 0.6f + happy * 0.4f;

    } else if (sad > 0.35f) {
        /* triste */
        t->left_eye.openness  = 0.70f;
        t->right_eye.openness = 0.70f;
        t->left_eye.expr_lid  = 0.30f;
        t->right_eye.expr_lid = 0.30f;
        t->mouth.shape        = MOUTH_FROWN;
        t->mouth.width        = 0.35f;
        t->mouth.curvature    = 0.60f;

    } else if (curio > 0.30f) {
        /* curioso — olhos assimétricos */
        t->left_eye.openness  = 1.10f;
        t->right_eye.openness = 0.80f;
        t->mouth.shape        = MOUTH_NEUTRAL;
        t->mouth.width        = 0.30f;

    } else {
        /* neutro */
        t->mouth.shape     = MOUTH_NEUTRAL;
        t->mouth.width     = 0.40f;
        t->mouth.curvature = 0.50f;
    }

    /* desvio de pupila pelo nível de curiosidade/estresse */
    float px = (float)(drives->curiosity - 0.5) * 0.4f;
    float py = (float)(drives->stress    - 0.5) * -0.25f;
    t->left_eye.pupil_x  = a->pupil_tx + px;
    t->right_eye.pupil_x = a->pupil_tx + px;
    t->left_eye.pupil_y  = a->pupil_ty + py;
    t->right_eye.pupil_y = a->pupil_ty + py;
}

void anim_update(AnimState *a, double dt) {
    /* ── piscar ── */
    if (a->blinking) {
        a->blink_phase += dt * PI / 0.12;   /* ciclo completo em 240 ms */
        if (a->blink_phase >= PI) {
            a->blinking    = 0;
            a->blink_phase = 0.0;
            a->blink_lid   = 0.f;
        } else {
            a->blink_lid = (float)sin(a->blink_phase);
        }
    } else {
        a->blink_timer -= dt;
        if (a->blink_timer <= 0.0) {
            a->blinking    = 1;
            a->blink_phase = 0.0;
            a->blink_timer = 2.5 + 3.5 * ((double)rand() / RAND_MAX);
        }
    }

    /* ── flutuação vertical ── */
    a->bob_phase += dt * 0.55;              /* ~0.55 Hz */
    a->bob_y = (int)roundf(3.5f * (float)sin(a->bob_phase));

    /* ── desvio ocioso da pupila ── */
    a->pupil_timer -= dt;
    if (a->pupil_timer <= 0.0) {
        float r = (float)rand() / RAND_MAX;
        float ang = r * 2.f * (float)PI;
        float mag = 0.15f + 0.20f * (float)rand() / RAND_MAX;
        a->pupil_tx = mag * cosf(ang);
        a->pupil_ty = mag * sinf(ang);
        a->pupil_timer = 1.5 + 3.0 * ((double)rand() / RAND_MAX);
    }

    /* ── interpolar current → target ── */
    float alpha = (float)(1.0 - exp(-5.0 * dt));  /* ~5 rad/s */

    lerp_eye  (&a->current.left_eye,  &a->target.left_eye,  alpha);
    lerp_eye  (&a->current.right_eye, &a->target.right_eye, alpha);
    lerp_mouth(&a->current.mouth,     &a->target.mouth,     alpha);
    (void)smooth;
}
