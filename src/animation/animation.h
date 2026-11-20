#ifndef CORTEX_ANIMATION_H
#define CORTEX_ANIMATION_H

#include "face.h"

/* Forward declaration — neuro.h está no root */
typedef struct Brain Brain;
typedef struct { double energy, stress, curiosity, satiety, affection; } Drives;
typedef struct { double valence, arousal, dominance, mood; int current_emotion; } Emotion;

typedef struct {
    FaceState  current;    /* estado renderizado agora */
    FaceState  target;     /* alvo para interpolação */

    /* piscar */
    double blink_timer;    /* tempo até próximo piscar */
    double blink_phase;    /* 0..π durante piscar */
    int    blinking;
    float  blink_lid;      /* 0..1 calculado de blink_phase */

    /* flutuação */
    double bob_phase;      /* avança em tempo real */
    int    bob_y;          /* pixels inteiros, -4..+4 */

    /* desvio ocioso da pupila */
    double pupil_timer;
    float  pupil_tx, pupil_ty;   /* alvo */
} AnimState;

void anim_init   (AnimState *a);
void anim_update (AnimState *a, double dt);

/*
 * Lê saídas motoras do cérebro SNN e define anim->target.
 * Chame UMA vez por frame, antes de anim_update.
 */
void anim_sync_brain(AnimState *a,
                     const Brain *brain,
                     const Drives *drives,
                     const Emotion *emotion);

#endif
