#include "bridge_apply.h"
#include <string.h>

void bridge_apply(BridgeState *bs, AnimState *anim, bool *bridge_override) {
    *bridge_override = false;

    /* ── drives ── */
    if (bs->dirty_drives) {
        /* homeostasis do pet é sobrescrita — Cortex assume */
        bs->dirty_drives = false;
        /* registrado na animação via emotion abaixo */
    }

    /* ── emoção global ── */
    if (bs->dirty_emotion) {
        float v = bs->emotion.valence;
        float a = bs->emotion.arousal;

        /* mapeia emoção para alvo de expressão sem passar pelo SNN */
        FaceState *t = &anim->target;

        /* valence → boca */
        if (v > 0.4f) {
            t->mouth.shape     = MOUTH_SMILE;
            t->mouth.curvature = v;
            t->mouth.width     = 0.4f + v * 0.4f;
        } else if (v < -0.4f) {
            t->mouth.shape     = MOUTH_FROWN;
            t->mouth.curvature = -v;
            t->mouth.width     = 0.35f + (-v) * 0.3f;
        } else {
            t->mouth.shape     = MOUTH_NEUTRAL;
            t->mouth.width     = 0.35f;
        }

        /* arousal → abertura dos olhos */
        float open = 0.7f + a * 0.55f;
        t->left_eye.openness  = open;
        t->right_eye.openness = open;

        bs->dirty_emotion  = false;
        *bridge_override   = true;
    }

    /* ── olho esquerdo ── */
    if (bs->dirty_eye_left) {
        EyeState *e = &anim->target.left_eye;
        e->openness = bs->eye[0].openness;
        e->pupil_x  = bs->eye[0].pupil_x;
        e->pupil_y  = bs->eye[0].pupil_y;
        bs->dirty_eye_left = false;
        *bridge_override   = true;
    }

    /* ── olho direito ── */
    if (bs->dirty_eye_right) {
        EyeState *e = &anim->target.right_eye;
        e->openness = bs->eye[1].openness;
        e->pupil_x  = bs->eye[1].pupil_x;
        e->pupil_y  = bs->eye[1].pupil_y;
        bs->dirty_eye_right = false;
        *bridge_override    = true;
    }

    /* ── boca ── */
    if (bs->dirty_mouth) {
        MouthState *m  = &anim->target.mouth;
        m->shape       = (MouthShape)bs->mouth.shape;
        m->curvature   = bs->mouth.curvature;
        bs->dirty_mouth  = false;
        *bridge_override = true;
    }

    /* ── flutuação ── */
    if (bs->dirty_bob) {
        anim->bob_freq_hz  = bs->bob.freq_hz;
        anim->bob_amp_px   = bs->bob.amplitude_px;
        bs->dirty_bob      = false;
    }

    /* ── piscar forçado ── */
    if (bs->trigger_blink && !anim->blinking) {
        anim->blinking    = 1;
        anim->blink_phase = 0.0;
        bs->trigger_blink = false;
    }
}
