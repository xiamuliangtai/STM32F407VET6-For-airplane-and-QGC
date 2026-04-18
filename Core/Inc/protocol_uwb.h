#ifndef __PROTOCOL_UWB_H
#define __PROTOCOL_UWB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void ProtocolUWB_Init(void);
void ProtocolUWB_ParseBytes(const uint8_t *data, uint16_t len);
uint8_t ProtocolUWB_IsDataFresh(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_UWB_H */
