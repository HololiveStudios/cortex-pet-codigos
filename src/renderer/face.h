#ifndef CORTEX_FACE_H
#define CORTEX_FACE_H

#include "pixel_draw.h"

typedef enum {
    MOUTH_NEUTRAL = 0,
    MOUTH_SMILE,
    MOUTH_FROWN,
    MOUTH_OPEN,
    MOUTH_WORRIED,
} MouthShape;

typedef struct {
    float openness;   /* 0=fechado, 1=normal, 1.2=arregalado */
    float pupil_x;    /* -1..+1, offset horizontal da pupila */
    float pupil_y;    /* -1..+1, offset vertical da pupila */
    float expr_lid;   /* 0=aberto, 1=fechado (expressão: triste/sonolento) */
} EyeState;

typedef struct {
    MouthShape shape;
    float openness;   /* 0..1, usado em MOUTH_OPEN */
    float width;      /* 0..1, multiplica largura base */
    float curvature;  /* 0..1, profundidade do arco */
} MouthState;

typedef struct {
    EyeState  left_eye;
    EyeState  right_eye;
    MouthState mouth;
    float     tilt;    /* reservado — inclinação futura */
} FaceState;

/*
 * Desenha o personagem no canvas.
 * bob_y:     offset vertical de flutuação (pixels inteiros, tipicamente -4..+4)
 * blink_lid: 0=olhos abertos, 1=olhos fechados (animação de piscar)
 */
void face_draw(Canvas buf, const FaceState *face, int bob_y, float blink_lid);

/* Preenche um FaceState com valores padrão (neutro). */
void face_default(FaceState *face);

#endif
