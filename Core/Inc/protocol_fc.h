#ifndef __PROTOCOL_FC_H
#define __PROTOCOL_FC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void ProtocolFC_Init(void);
void ProtocolFC_ParseBytes(const uint8_t *data, uint16_t len);
void ProtocolFC_SendTakeoff(float z);
void ProtocolFC_SendPositionTarget(float x, float y, float z, float yaw);
void ProtocolFC_SendLand(void);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_FC_H */
