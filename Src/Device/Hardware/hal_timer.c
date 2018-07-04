#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "sysctl.h"
#include "interrupt.h"
#include "timer.h"

#include "hal_timer.h"

#include "Messages.h"
#include "CommandTask.h"

#define ENTER_CRITICAL_REGION_QUICK()   IntMasterDisable()
#define LEAVE_CRITICAL_REGION_QUICK()   IntMasterEnable()

static volatile unsigned char TimerInUseMask = 0;
static void TimerIsr(void);

void InitializeTimer(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet() / 32);  // 32 Hz
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
//    TimerEnable(TIMER1_BASE, TIMER_A);
}

void Timer1IntHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerIsr();
}

static void TimerIsr(void)
{
    if ( QueryTimerUserActive(TIMER_BUTTON) ) {
        RouteCommandFromIsr(ButtonState);
    }
}

void EnableTimerUser(unsigned char UserMask)
{
    ENTER_CRITICAL_REGION_QUICK();

    // If not currently enabled, enable the timer ISR
    if ( TimerInUseMask == 0 ) {
        TimerEnable(TIMER1_BASE, TIMER_A);
    }

    TimerInUseMask |= UserMask;

    LEAVE_CRITICAL_REGION_QUICK();
}


void DisableTimerUser(unsigned char UserMask)
{

    ENTER_CRITICAL_REGION_QUICK();

    TimerInUseMask &= ~UserMask;

    // If there are no more users, disable the interrupt
    if ( TimerInUseMask == 0 ) {
        TimerDisable(TIMER1_BASE, TIMER_A);
    }

    LEAVE_CRITICAL_REGION_QUICK();
}

unsigned char QueryTimerUserActive(unsigned char UserMask)
{
  return TimerInUseMask & UserMask;
}
