#include "window.h"
#include "config.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KEY_BUF 64

struct Window {
    GLFWwindow *handle;
    /* drag sem barra de título */
    bool  dragging;
    double drag_ox, drag_oy;
    int   win_ox, win_oy;
    /* fila circular de chars */
    char  key_buf[KEY_BUF];
    int   key_head, key_tail;
};

static struct Window *g_win = NULL;

/* ── callbacks ── */
static void cb_mouse_btn(GLFWwindow *gw, int btn, int act, int mods) {
    (void)mods;
    if (!g_win || btn != GLFW_MOUSE_BUTTON_LEFT) return;
    if (act == GLFW_PRESS) {
        g_win->dragging = true;
        glfwGetCursorPos(gw, &g_win->drag_ox, &g_win->drag_oy);
        glfwGetWindowPos(gw, &g_win->win_ox, &g_win->win_oy);
    } else {
        g_win->dragging = false;
    }
}

static void cb_cursor(GLFWwindow *gw, double x, double y) {
    if (!g_win || !g_win->dragging) return;
    int nx = g_win->win_ox + (int)(x - g_win->drag_ox);
    int ny = g_win->win_oy + (int)(y - g_win->drag_oy);
    glfwSetWindowPos(gw, nx, ny);
}

static void cb_key(GLFWwindow *gw, int key, int sc, int act, int mods) {
    (void)sc; (void)mods;
    if (key == GLFW_KEY_ESCAPE && act == GLFW_PRESS)
        glfwSetWindowShouldClose(gw, GLFW_TRUE);
}

static void cb_char(GLFWwindow *gw, unsigned int cp) {
    (void)gw;
    if (!g_win || cp >= 128) return;
    g_win->key_buf[g_win->key_head % KEY_BUF] = (char)cp;
    g_win->key_head++;
}

/* ── public ── */
Window *window_create(const char *title, int w, int h) {
    if (!glfwInit()) return NULL;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED,  GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING,   GLFW_TRUE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES,    0);

    GLFWwindow *handle = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!handle) { glfwTerminate(); return NULL; }

    glfwMakeContextCurrent(handle);

    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwDestroyWindow(handle);
        glfwTerminate();
        return NULL;
    }

    Window *win = calloc(1, sizeof(Window));
    win->handle = handle;
    g_win = win;

    glfwSetMouseButtonCallback(handle, cb_mouse_btn);
    glfwSetCursorPosCallback(handle,   cb_cursor);
    glfwSetKeyCallback(handle,         cb_key);
    glfwSetCharCallback(handle,        cb_char);
    glfwSwapInterval(1);

    /* posiciona no canto inferior-direito */
    GLFWmonitor *mon = glfwGetPrimaryMonitor();
    if (mon) {
        const GLFWvidmode *mode = glfwGetVideoMode(mon);
        if (mode) glfwSetWindowPos(handle, mode->width - w - 20, mode->height - h - 60);
    }

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE,       GL_ONE_MINUS_SRC_ALPHA);
    return win;
}

void window_destroy(Window *w) {
    if (!w) return;
    glfwDestroyWindow(w->handle);
    glfwTerminate();
    free(w);
}

bool window_should_close(const Window *w) {
    return w && glfwWindowShouldClose(w->handle);
}

void window_poll_events(Window *w) { (void)w; glfwPollEvents(); }

void window_swap_buffers(Window *w) {
    if (w) glfwSwapBuffers(w->handle);
}

char window_read_char(Window *w) {
    if (!w || w->key_head == w->key_tail) return 0;
    char c = w->key_buf[w->key_tail % KEY_BUF];
    w->key_tail++;
    return c;
}
