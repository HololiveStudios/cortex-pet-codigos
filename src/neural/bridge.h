#ifndef CORTEX_BRIDGE_H
#define CORTEX_BRIDGE_H

#include <stdbool.h>
#include "protocol.h"
#include "../renderer/face.h"

/*
 * BridgeState acumula os comandos recebidos via IPC neste frame.
 * O render loop lê e aplica ao AnimState.
 * Campos com dirty=true foram atualizados desde o último consume.
 */
typedef struct {
    bool dirty_emotion;
    CPayloadEmotion emotion;

    bool dirty_eye_left;
    bool dirty_eye_right;
    CPayloadEye eye[2];        /* [0]=esquerdo [1]=direito */

    bool dirty_mouth;
    CPayloadMouth mouth;

    bool dirty_bob;
    CPayloadBob bob;

    bool dirty_drives;
    CPayloadDrives drives;

    bool trigger_blink;

    /* feedback p/ resposta de STATUS */
    CPayloadStatus last_status;
} BridgeState;

typedef struct Bridge Bridge;

Bridge      *bridge_create(void);
void         bridge_destroy(Bridge *b);

/*
 * Chame uma vez por frame no loop principal.
 * Lê todos os pacotes pendentes no socket sem bloquear,
 * acumula em state, e envia respostas síncronas (STATUS, PONG).
 */
void         bridge_poll(Bridge *b, BridgeState *state);

/*
 * Atualiza last_status antes de qualquer bridge_poll que possa
 * responder um GET_STATUS.
 */
void         bridge_update_status(Bridge *b, const CPayloadStatus *s);

/* true se pelo menos um cliente está conectado */
bool         bridge_has_client(const Bridge *b);

#endif /* CORTEX_BRIDGE_H */
