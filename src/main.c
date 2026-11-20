#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "config.h"
#include "window.h"
#include "face.h"
#include "framebuffer.h"
#include "animation.h"

/* motor SNN — headers no root do projeto */
#include "neuro.h"
#include "homeostasis.h"
#include "behavior.h"
#include "conditioning.h"
#include "memory.h"
#include "sensors.h"
#include "runtime_types.h"

#include <glad/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__unix__) || defined(__APPLE__)
#  include <unistd.h>
static void sleep_ms(int ms) {
    struct timespec ts = { ms/1000, (long)(ms%1000)*1000000L };
    nanosleep(&ts, NULL);
}
#else
#  include <windows.h>
static void sleep_ms(int ms) { Sleep(ms); }
#endif

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
    srand((unsigned)time(NULL));

    /* ── janela e renderer ── */
    Window *win = window_create("Cortex Pet", WINDOW_W, WINDOW_H);
    if (!win) { fprintf(stderr, "window_create falhou\n"); return 1; }

    Framebuffer *fb = fb_create();
    if (!fb)  { fprintf(stderr, "fb_create falhou\n"); return 1; }

    /* ── motor SNN ── */
    Brain *brain = brain_create(42, "default");
    if (!brain) { fprintf(stderr, "brain_create falhou\n"); return 1; }

    Drives          drives;
    Emotion         emotion;
    FaceState       face_legacy;   /* usado só como ponte p/ behavior_ */
    Behavior        behavior;
    ConditioningState cond;
    EpisodeMemory   memory;
    Sensors        *sensors = sensors_open();

    homeostasis_init(&drives);
    behavior_init(&behavior, &emotion, &face_legacy);
    conditioning_init(&cond);
    memory_init(&memory);

    /* ── animação ── */
    AnimState anim;
    anim_init(&anim);

    static Canvas canvas;   /* 128×128, estático — evita stack grande */

    const double TARGET_DT = 1.0 / 60.0;
    double t_last = now_seconds();
    double sim_t  = 0.0;

    while (!window_should_close(win)) {

        double t_now = now_seconds();
        double dt    = t_now - t_last;
        if (dt > 0.1) dt = 0.1;   /* cap p/ não explodir no debug */
        t_last = t_now;
        sim_t += dt;

        window_poll_events(win);

        /* ── input do usuário (mesmo mapeamento do terminal) ── */
        char key;
        while ((key = window_read_char(win)) != 0) {
            switch (key | 0x20) {   /* lowercase */
            case 'c':
                homeostasis_event(&drives, "caress");
                conditioning_reward(&cond, 0.3);
                memory_record(&memory, "caress", sim_t);
                brain_learn_stdp(brain, 0.8, dt);
                break;
            case 'p':
                homeostasis_event(&drives, "poke");
                conditioning_punish(&cond, 0.4);
                memory_record(&memory, "poke", sim_t);
                brain_learn_stdp(brain, -0.6, dt);
                break;
            case 'f':
                homeostasis_event(&drives, "feed");
                conditioning_reward(&cond, 0.4);
                memory_record(&memory, "feed", sim_t);
                break;
            case 't':
                homeostasis_event(&drives, "talk");
                break;
            case 'r':
                brain_reset_habituation(brain);
                break;
            }
        }

        /* ── sensores + SNN ── */
        double inputs[PESTINHA_CHANNELS] = {0};
        SensorReading sr = {0};
        if (sensors) sensors_poll(sensors, sim_t, &sr, inputs);

        brain_stimulate(brain, inputs);
        brain_step(brain, dt * 1000.0);
        brain_learn_stdp(brain, cond.reward_trace - cond.punishment_trace, dt);

        homeostasis_update(&drives, &behavior, dt,
                           behavior.wandering ? 1 : 0,
                           emotion.arousal, inputs);
        behavior_update(&behavior, &drives, &cond, brain, sim_t, dt);
        behavior_update_emotion(&emotion, &behavior, &face_legacy, brain, sim_t, dt);
        conditioning_update(&cond, dt);

        /* ── animação ── */
        anim_sync_brain(&anim, brain, &drives, &emotion);
        anim_update(&anim, dt);

        /* ── render ── */
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);

        face_draw(canvas, &anim.current, anim.bob_y, anim.blink_lid);
        fb_present(fb, canvas);

        window_swap_buffers(win);

        /* throttle se vsync não funcionar */
        double elapsed = now_seconds() - t_now;
        if (elapsed < TARGET_DT)
            sleep_ms((int)((TARGET_DT - elapsed) * 1000.0));
    }

    sensors_close(sensors);
    brain_destroy(brain);
    fb_destroy(fb);
    window_destroy(win);
    return 0;
}
