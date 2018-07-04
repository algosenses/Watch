#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "sysctl.h"
#include "interrupt.h"
#include "gpio.h"
#include "timer.h"

#include "Messages.h"
#include "OneSecondTimers.h"
#include "BufferPool.h"

#include "hal_rtc.h"

static volatile unsigned int Year;
static volatile unsigned char Month;
static volatile unsigned char DayOfMonth;
static volatile unsigned char DayOfWeek;
static volatile unsigned char Hour;
static volatile unsigned char Minute;
static volatile unsigned char Second;

static void IncreaseSeconds(void)
{
    Second++;
    if (Second > 59) {
        Second = 0;
        Minute++;
        if (Minute > 59) {
            Minute = 0;
            Hour++;
            if (Hour >= 24) {
                Hour = 0;
                DayOfWeek++;
                if (DayOfWeek >= 7) {
                    DayOfWeek = 0;
                    DayOfMonth++;
                    if (DayOfMonth >= 32) {
                        DayOfMonth = 1;
                        Month++;
                    }
                    if (Month >= 13) {
                        Month = 1;
                        Year++;
                    }
                }
            }
        }

    }
}

void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    IncreaseSeconds();

    OneSecondTimerHandlerIsr();
}

void InitializeRealTimeClock( void )
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    Second = 0;
    Minute = 0;
    Hour = 0;
    DayOfWeek = 0;
    DayOfMonth = 1;
    Month = 1;
    Year = 1970;
}

int SetRTCYEAR(int year)
{
    Year = year;
    return 0;
}

int SetRTCMON(int month)
{
    Month = month;
    return 0;
}

int SetRTCDAY(int day)
{
    DayOfMonth = day;
    return 0;
}

int SetRTCDOW(int day)
{
    DayOfWeek = day;
    return 0;
}

int SetRTCHOUR(int hour)
{
    Hour = hour;
    return 0;
}

int SetRTCMIN(int minute)
{
    Minute = minute;
    return 0;
}

int SetRTCSEC(int second)
{
    Second = second;
    return 0;
}

int GetRTCDATE(void)
{
    return 0;
}

int GetRTCYEAR(void)
{
    return Year;
}

int GetRTCMON(void)
{
    return Month;
}

int GetRTCDOW(void)
{
    return DayOfWeek;
}

int GetRTCDAY(void)
{
    return DayOfMonth;
}

int GetRTCHOUR(void)
{
    return Hour;
}

int GetRTCMIN(void)
{
    return Minute;
}

int GetRTCSEC(void)
{
    return Second;
}

void halRtcSet(tRtcHostMsgPayload* pRtcData)
{
    tWordByteUnion temp;
    temp.bytes.byte0 = pRtcData->YearLsb;
    temp.bytes.byte1 = pRtcData->YearMsb;
    SetRTCYEAR((unsigned int)(temp.word));
    SetRTCMON((unsigned int)(pRtcData->Month));
    SetRTCDAY((unsigned int)(pRtcData->DayOfMonth));
    SetRTCDOW((unsigned int)(pRtcData->DayOfWeek));
    SetRTCHOUR((unsigned int)(pRtcData->Hour));
    SetRTCMIN((unsigned int)(pRtcData->Minute));
    SetRTCSEC((unsigned int)(pRtcData->Second));
}

void halRtcGet(tRtcHostMsgPayload* pRtcData)
{
    tWordByteUnion temp;
    temp.word = GetRTCYEAR();
    pRtcData->YearLsb = temp.bytes.byte0;
    pRtcData->YearMsb = temp.bytes.byte1;
    pRtcData->Month = GetRTCMON();
    pRtcData->DayOfMonth = GetRTCDAY();
    pRtcData->DayOfWeek = GetRTCDOW();
    pRtcData->Hour = GetRTCHOUR();
    pRtcData->Minute = GetRTCMIN();
    pRtcData->Second = GetRTCSEC();
}
