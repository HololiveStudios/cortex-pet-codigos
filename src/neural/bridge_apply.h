#ifndef CORTEX_BRIDGE_APPLY_H
#define CORTEX_BRIDGE_APPLY_H

#include "bridge.h"
#include "../animation/animation.h"

/*
 * Aplica os comandos recebidos via IPC ao estado de animação.
 * Chame após bridge_poll e antes de anim_sync_brain.
 *
 * Se bridge_override==true após o retorno, anim_sync_brain
 * deve ser pulado (Cortex assumiu controle total).
 */
void bridge_apply(BridgeState *bs, AnimState *anim, bool *bridge_override);

#endif
