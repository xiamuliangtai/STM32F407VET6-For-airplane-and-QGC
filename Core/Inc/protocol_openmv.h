#ifndef __PROTOCOL_OPENMV_H
#define __PROTOCOL_OPENMV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void ProtocolOpenMV_Init(void);
void ProtocolOpenMV_ParseBytes(const uint8_t *data, uint16_t len);
uint8_t ProtocolOpenMV_IsDataFresh(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_OPENMV_H */
