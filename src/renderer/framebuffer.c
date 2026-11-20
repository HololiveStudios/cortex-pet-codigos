#include "framebuffer.h"
#include <glad/gl.h>
#include <stdlib.h>
#include <stdio.h>

/* ── shaders embutidos ── */
static const char *VERT =
    "#version 330 core\n"
    "layout(location=0) in vec2 a_pos;\n"
    "layout(location=1) in vec2 a_uv;\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "    v_uv = a_uv;\n"
    "}\n";

static const char *FRAG =
    "#version 330 core\n"
    "in  vec2 v_uv;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D u_tex;\n"
    "void main() {\n"
    "    frag_color = texture(u_tex, v_uv);\n"
    "}\n";

/*
 *  Quad NDC com UV mapeado para:
 *    canto superior-esquerdo  NDC (-1, +1) → UV (0,0) = pixel (0,0) do canvas
 *    canto inferior-direito   NDC (+1, -1) → UV (1,1) = pixel (W-1, H-1)
 */
static const float QUAD[16] = {
    /* pos.x  pos.y    uv.x  uv.y */
    -1.f,  1.f,    0.f, 0.f,
     1.f,  1.f,    1.f, 0.f,
    -1.f, -1.f,    0.f, 1.f,
     1.f, -1.f,    1.f, 1.f,
};

struct Framebuffer {
    GLuint prog;
    GLuint vao, vbo;
    GLuint tex;
    GLint  u_tex;
};

static GLuint compile(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "[shader] %s\n", log);
        glDeleteShader(s); return 0;
    }
    return s;
}

Framebuffer *fb_create(void) {
    Framebuffer *fb = calloc(1, sizeof(*fb));

    GLuint vs = compile(GL_VERTEX_SHADER,   VERT);
    GLuint fs = compile(GL_FRAGMENT_SHADER, FRAG);
    fb->prog  = glCreateProgram();
    glAttachShader(fb->prog, vs);
    glAttachShader(fb->prog, fs);
    glLinkProgram(fb->prog);
    glDeleteShader(vs); glDeleteShader(fs);

    fb->u_tex = glGetUniformLocation(fb->prog, "u_tex");

    /* VAO/VBO */
    glGenVertexArrays(1, &fb->vao);
    glGenBuffers(1, &fb->vbo);
    glBindVertexArray(fb->vao);
    glBindBuffer(GL_ARRAY_BUFFER, fb->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glBindVertexArray(0);

    /* textura 128×128 RGBA8, nearest */
    glGenTextures(1, &fb->tex);
    glBindTexture(GL_TEXTURE_2D, fb->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 CANVAS_W, CANVAS_H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    return fb;
}

void fb_destroy(Framebuffer *fb) {
    if (!fb) return;
    glDeleteTextures(1, &fb->tex);
    glDeleteBuffers(1, &fb->vbo);
    glDeleteVertexArrays(1, &fb->vao);
    glDeleteProgram(fb->prog);
    free(fb);
}

void fb_present(Framebuffer *fb, const Canvas pixels) {
    /* upload CPU → GPU */
    glBindTexture(GL_TEXTURE_2D, fb->tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    CANVAS_W, CANVAS_H,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    /* draw quad */
    glUseProgram(fb->prog);
    glUniform1i(fb->u_tex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb->tex);
    glBindVertexArray(fb->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
