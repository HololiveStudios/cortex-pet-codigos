#include "bridge.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ════════════════════════════════════════════
   Implementação multiplataforma
   ════════════════════════════════════════════ */

#if defined(_WIN32)
/* ── Windows: Named Pipe ── */
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  define PIPE_NAME "\\\\.\\pipe\\cortex_pet"
#  define MAX_CLIENTS 4

struct Bridge {
    HANDLE   pipes[MAX_CLIENTS];
    OVERLAPPED ov[MAX_CLIENTS];
    int      count;
    CPayloadStatus status;
};

Bridge *bridge_create(void) {
    Bridge *b = calloc(1, sizeof(*b));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        b->pipes[i] = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
            MAX_CLIENTS,
            256, 256, 0, NULL
        );
        if (b->pipes[i] == INVALID_HANDLE_VALUE) {
            /* só o slot 0 é obrigatório */
            if (i == 0) { free(b); return NULL; }
            break;
        }
        b->count = i + 1;
        ConnectNamedPipe(b->pipes[i], &b->ov[i]);
    }
    return b;
}

void bridge_destroy(Bridge *b) {
    if (!b) return;
    for (int i = 0; i < b->count; i++) {
        DisconnectNamedPipe(b->pipes[i]);
        CloseHandle(b->pipes[i]);
    }
    free(b);
}

bool bridge_has_client(const Bridge *b) { (void)b; return true; }

static void win_send(HANDLE h, uint8_t type,
                     const void *payload, uint8_t plen) {
    uint8_t buf[256];
    buf[0] = CPROT_MAGIC0;
    buf[1] = CPROT_MAGIC1;
    buf[2] = type;
    buf[3] = plen;
    if (plen && payload) memcpy(buf + 4, payload, plen);
    DWORD written;
    WriteFile(h, buf, 4 + plen, &written, NULL);
}

static void win_process_packet(HANDLE h, const uint8_t *buf, int len,
                                BridgeState *state,
                                const CPayloadStatus *status) {
    if (len < CPROT_HDR_LEN) return;
    if (buf[0] != CPROT_MAGIC0 || buf[1] != CPROT_MAGIC1) return;
    uint8_t type = buf[2];
    uint8_t plen = buf[3];
    const uint8_t *pay = buf + CPROT_HDR_LEN;

    switch (type) {
    case CPROT_SET_EMOTION:
        if (plen >= sizeof(CPayloadEmotion)) {
            memcpy(&state->emotion, pay, sizeof(CPayloadEmotion));
            state->dirty_emotion = true;
        } break;
    case CPROT_SET_EYE:
        if (plen >= sizeof(CPayloadEye)) {
            CPayloadEye e; memcpy(&e, pay, sizeof(e));
            if (e.eye == 0 || e.eye == 2) { state->eye[0]=e; state->dirty_eye_left=true; }
            if (e.eye == 1 || e.eye == 2) { state->eye[1]=e; state->dirty_eye_right=true; }
        } break;
    case CPROT_SET_MOUTH:
        if (plen >= sizeof(CPayloadMouth)) {
            memcpy(&state->mouth, pay, sizeof(CPayloadMouth));
            state->dirty_mouth = true;
        } break;
    case CPROT_SET_BOB:
        if (plen >= sizeof(CPayloadBob)) {
            memcpy(&state->bob, pay, sizeof(CPayloadBob));
            state->dirty_bob = true;
        } break;
    case CPROT_TRIGGER_BLINK:
        state->trigger_blink = true; break;
    case CPROT_SET_DRIVES:
        if (plen >= sizeof(CPayloadDrives)) {
            memcpy(&state->drives, pay, sizeof(CPayloadDrives));
            state->dirty_drives = true;
        } break;
    case CPROT_GET_STATUS:
        win_send(h, CPROT_STATUS, status, sizeof(*status)); break;
    case CPROT_PING:
        win_send(h, CPROT_PONG, NULL, 0); break;
    }
}

void bridge_poll(Bridge *b, BridgeState *state) {
    for (int i = 0; i < b->count; i++) {
        uint8_t buf[256];
        DWORD bytes_read;
        if (ReadFile(b->pipes[i], buf, sizeof(buf), &bytes_read, NULL) && bytes_read > 0) {
            win_process_packet(b->pipes[i], buf, (int)bytes_read, state, &b->status);
        }
    }
}

void bridge_update_status(Bridge *b, const CPayloadStatus *s) {
    if (b && s) b->status = *s;
}

#else
/* ── Unix: Unix Domain Socket ── */
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <errno.h>

#  define SOCK_PATH   "/tmp/cortex_pet.sock"
#  define MAX_CLIENTS 8
#  define RECV_BUF    512

struct Bridge {
    int srv_fd;
    int clients[MAX_CLIENTS];
    int n_clients;
    CPayloadStatus status;
    /* buffer de leitura por cliente */
    uint8_t rbuf[MAX_CLIENTS][RECV_BUF];
    int     rlen[MAX_CLIENTS];
};

static void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Bridge *bridge_create(void) {
    Bridge *b = calloc(1, sizeof(*b));
    for (int i = 0; i < MAX_CLIENTS; i++) b->clients[i] = -1;

    b->srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (b->srv_fd < 0) { free(b); return NULL; }

    set_nonblock(b->srv_fd);
    unlink(SOCK_PATH);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);

    if (bind(b->srv_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ||
        listen(b->srv_fd, MAX_CLIENTS) < 0) {
        close(b->srv_fd);
        free(b);
        return NULL;
    }

    return b;
}

void bridge_destroy(Bridge *b) {
    if (!b) return;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (b->clients[i] >= 0) close(b->clients[i]);
    close(b->srv_fd);
    unlink(SOCK_PATH);
    free(b);
}

bool bridge_has_client(const Bridge *b) {
    return b && b->n_clients > 0;
}

static void unix_send(int fd, uint8_t type,
                      const void *payload, uint8_t plen) {
    uint8_t buf[256];
    buf[0] = CPROT_MAGIC0;
    buf[1] = CPROT_MAGIC1;
    buf[2] = type;
    buf[3] = plen;
    if (plen && payload) memcpy(buf + 4, payload, plen);
    write(fd, buf, (size_t)(4 + plen));
}

static void process_packet(int fd, const uint8_t *buf, int len,
                            BridgeState *state,
                            const CPayloadStatus *status) {
    while (len >= CPROT_HDR_LEN) {
        if (buf[0] != CPROT_MAGIC0 || buf[1] != CPROT_MAGIC1) {
            /* dessincronizado — descarta byte e tenta re-sync */
            buf++; len--;
            continue;
        }
        uint8_t type = buf[2];
        uint8_t plen = buf[3];

        if (len < CPROT_HDR_LEN + plen) break;   /* pacote incompleto */

        const uint8_t *pay = buf + CPROT_HDR_LEN;

        switch (type) {
        case CPROT_SET_EMOTION:
            if (plen >= (uint8_t)sizeof(CPayloadEmotion)) {
                memcpy(&state->emotion, pay, sizeof(CPayloadEmotion));
                state->dirty_emotion = true;
            } break;

        case CPROT_SET_EYE:
            if (plen >= (uint8_t)sizeof(CPayloadEye)) {
                CPayloadEye e; memcpy(&e, pay, sizeof(e));
                if (e.eye == 0 || e.eye == 2) {
                    state->eye[0] = e; state->dirty_eye_left  = true;
                }
                if (e.eye == 1 || e.eye == 2) {
                    state->eye[1] = e; state->dirty_eye_right = true;
                }
            } break;

        case CPROT_SET_MOUTH:
            if (plen >= (uint8_t)sizeof(CPayloadMouth)) {
                memcpy(&state->mouth, pay, sizeof(CPayloadMouth));
                state->dirty_mouth = true;
            } break;

        case CPROT_SET_BOB:
            if (plen >= (uint8_t)sizeof(CPayloadBob)) {
                memcpy(&state->bob, pay, sizeof(CPayloadBob));
                state->dirty_bob = true;
            } break;

        case CPROT_TRIGGER_BLINK:
            state->trigger_blink = true;
            break;

        case CPROT_SET_DRIVES:
            if (plen >= (uint8_t)sizeof(CPayloadDrives)) {
                memcpy(&state->drives, pay, sizeof(CPayloadDrives));
                state->dirty_drives = true;
            } break;

        case CPROT_GET_STATUS:
            unix_send(fd, CPROT_STATUS, status, sizeof(*status));
            break;

        case CPROT_PING:
            unix_send(fd, CPROT_PONG, NULL, 0);
            break;
        }

        buf += CPROT_HDR_LEN + plen;
        len -= CPROT_HDR_LEN + plen;
    }
}

void bridge_poll(Bridge *b, BridgeState *state) {
    /* aceitar novos clientes */
    for (;;) {
        int fd = accept(b->srv_fd, NULL, NULL);
        if (fd < 0) break;   /* EAGAIN = sem mais clientes esperando */
        set_nonblock(fd);

        bool placed = false;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (b->clients[i] < 0) {
                b->clients[i] = fd;
                b->rlen[i]    = 0;
                b->n_clients++;
                placed = true;
                break;
            }
        }
        if (!placed) close(fd);
    }

    /* ler clientes existentes */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int fd = b->clients[i];
        if (fd < 0) continue;

        int room = RECV_BUF - b->rlen[i];
        if (room <= 0) {
            /* buffer cheio sem pacote válido — descarta */
            b->rlen[i] = 0;
            room = RECV_BUF;
        }

        ssize_t n = read(fd, b->rbuf[i] + b->rlen[i], (size_t)room);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            /* erro real → fecha cliente */
            close(fd);
            b->clients[i] = -1;
            b->rlen[i]    = 0;
            b->n_clients--;
            continue;
        }

        if (n == 0) {
            /* conexão encerrada pelo cliente */
            close(fd);
            b->clients[i] = -1;
            b->rlen[i]    = 0;
            b->n_clients--;
            continue;
        }

        b->rlen[i] += (int)n;
        process_packet(fd, b->rbuf[i], b->rlen[i], state, &b->status);

        /*
         * Move bytes não-consumidos para o início do buffer.
         * process_packet consome tudo que pode; o restante fica
         * aguardando o próximo read.
         */
        b->rlen[i] = 0;   /* versão simples: descarta sobra */
    }
}

void bridge_update_status(Bridge *b, const CPayloadStatus *s) {
    if (b && s) b->status = *s;
}
#endif /* _WIN32 */
