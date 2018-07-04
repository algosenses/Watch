#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "Defines.h"

#define TIMER_VIBRATION       ( BIT0 )
#define TIMER_RESERVED        ( BIT1 )
#define TIMER_BUTTON          ( BIT2 )
#define TIMER_PEDOMETER       ( BIT3 )

void InitializeTimer(void);
void EnableTimerUser(unsigned char UserMask);
void DisableTimerUser(unsigned char UserMask);
unsigned char QueryTimerUserActive(unsigned char UserMask);

#endif /* HAL_TIMER_H */
