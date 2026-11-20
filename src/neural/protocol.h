#ifndef CORTEX_PROTOCOL_H
#define CORTEX_PROTOCOL_H

#include <stdint.h>

/*
 * Protocolo binário sobre Unix Domain Socket (Linux/macOS)
 * ou Named Pipe (Windows).
 *
 * Socket path : /tmp/cortex_pet.sock
 * Named pipe  : \\.\pipe\cortex_pet
 *
 * Toda mensagem segue o layout:
 *
 *   [magic: 2 bytes] [type: 1 byte] [payload_len: 1 byte] [payload: 0..252 bytes]
 *
 * magic = 0xC0 0x7E  ("CO~" → Cortex)
 * Máximo de payload: 252 bytes (mensagem total <= 256 bytes).
 *
 * O servidor (pet) lê no sentido cliente → server.
 * Após cada comando que gera resposta, envia 1 pacote de volta.
 */

#define CPROT_MAGIC0  0xC0u
#define CPROT_MAGIC1  0x7Eu
#define CPROT_HDR_LEN 4

/* ── Tipos de mensagem (cliente → servidor) ── */
typedef enum {
    /*
     * SET_EMOTION  — força expressão emocional
     * payload (8 bytes):
     *   float valence    [-1..+1]
     *   float arousal    [ 0..+1]
     */
    CPROT_SET_EMOTION   = 0x01,

    /*
     * SET_EYE  — controla olhos individualmente
     * payload (12 bytes):
     *   uint8  eye          0=esquerdo 1=direito 2=ambos
     *   uint8  _pad[3]
     *   float  openness     [0..1.3]
     *   float  pupil_x      [-1..+1]
     *   float  pupil_y      [-1..+1]
     */
    CPROT_SET_EYE       = 0x02,

    /*
     * SET_MOUTH  — controla boca
     * payload (8 bytes):
     *   uint8  shape        0=NEUTRAL 1=SMILE 2=FROWN 3=OPEN 4=WORRIED
     *   uint8  _pad[3]
     *   float  curvature    [0..1]
     *   (openness e width mantidos pela animação)
     */
    CPROT_SET_MOUTH     = 0x03,

    /*
     * SET_BOB  — controla frequência e amplitude de flutuação
     * payload (8 bytes):
     *   float  freq_hz      [0.1..4.0]
     *   float  amplitude_px [0..8]
     */
    CPROT_SET_BOB       = 0x04,

    /*
     * TRIGGER_BLINK  — dispara piscar imediato
     * payload: vazio (0 bytes)
     */
    CPROT_TRIGGER_BLINK = 0x05,

    /*
     * SET_DRIVES  — atualiza valores homeostáticos
     * payload (20 bytes):
     *   float energy    [0..1]
     *   float stress    [0..1]
     *   float curiosity [0..1]
     *   float satiety   [0..1]
     *   float affection [0..1]
     */
    CPROT_SET_DRIVES    = 0x06,

    /*
     * GET_STATUS  — solicita telemetria atual
     * payload: vazio (0 bytes)
     * resposta: CPROT_STATUS
     */
    CPROT_GET_STATUS    = 0x10,

    /*
     * PING  — keepalive
     * payload: vazio (0 bytes)
     * resposta: CPROT_PONG
     */
    CPROT_PING          = 0xFE,
} CProtMsgType;

/* ── Tipos de mensagem (servidor → cliente) ── */
typedef enum {
    /*
     * STATUS  — resposta ao GET_STATUS
     * payload (28 bytes):
     *   float energy, stress, curiosity, satiety, affection
     *   float valence, arousal
     */
    CPROT_STATUS        = 0x80,

    CPROT_PONG          = 0xFF,
    CPROT_ERR           = 0x00,
} CProtRespType;

/* ── Structs de payload (uso direto com memcpy) ── */

typedef struct __attribute__((packed)) {
    float valence;
    float arousal;
} CPayloadEmotion;      /* 8 bytes */

typedef struct __attribute__((packed)) {
    uint8_t eye;
    uint8_t _pad[3];
    float   openness;
    float   pupil_x;
    float   pupil_y;
} CPayloadEye;          /* 16 bytes */

typedef struct __attribute__((packed)) {
    uint8_t shape;
    uint8_t _pad[3];
    float   curvature;
} CPayloadMouth;        /* 8 bytes */

typedef struct __attribute__((packed)) {
    float freq_hz;
    float amplitude_px;
} CPayloadBob;          /* 8 bytes */

typedef struct __attribute__((packed)) {
    float energy;
    float stress;
    float curiosity;
    float satiety;
    float affection;
} CPayloadDrives;       /* 20 bytes */

typedef struct __attribute__((packed)) {
    float energy;
    float stress;
    float curiosity;
    float satiety;
    float affection;
    float valence;
    float arousal;
} CPayloadStatus;       /* 28 bytes */

#endif /* CORTEX_PROTOCOL_H */
