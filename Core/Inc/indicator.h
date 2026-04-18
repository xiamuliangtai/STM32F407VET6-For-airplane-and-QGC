#ifndef __INDICATOR_H
#define __INDICATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void Indicator_Init(void);
void Indicator_Update(SystemContext_t *ctx);
void Indicator_SetLed(uint8_t on);

#ifdef __cplusplus
}
#endif

#endif /* __INDICATOR_H */
