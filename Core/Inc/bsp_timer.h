#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Timer_Init(void);
uint32_t BSP_Timer_NowMs(void);
uint8_t BSP_Timer_IsElapsed(uint32_t now, uint32_t start, uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TIMER_H */
