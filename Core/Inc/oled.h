#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t OLED_Init(void);
uint8_t OLED_IsReady(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowUInt(uint8_t x, uint8_t page, uint32_t value, uint8_t width);
void OLED_ShowTestScreen(uint32_t counter, uint8_t gs_online);
void OLED_ShowGsRxScreen(uint8_t gs_online,
                         uint8_t rx_status,
                         uint8_t rx_code,
                         uint8_t rx_seq,
                         uint8_t mission_valid,
                         uint8_t mission_seq,
                         uint8_t mission_points);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
