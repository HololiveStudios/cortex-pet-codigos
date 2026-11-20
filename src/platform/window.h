#ifndef CORTEX_WINDOW_H
#define CORTEX_WINDOW_H

#include <stdbool.h>

typedef struct Window Window;

Window *window_create(const char *title, int w, int h);
void    window_destroy(Window *w);
bool    window_should_close(const Window *w);
void    window_poll_events(Window *w);
void    window_swap_buffers(Window *w);

/* Lê próximo char digitado (0 = vazio). Substitui terminal_ui_read_key. */
char    window_read_char(Window *w);

#endif
