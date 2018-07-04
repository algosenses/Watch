#ifndef HAL_RTC_H
#define HAL_RTC_H

void InitializeRealTimeClock( void );

/*! Sets the RTC
 *
 * \note There could be a separate struct to abstract this, however the message
 *  data was laid out to exactly match the MSP430 RTC registers.  The asm level
 *  patch functions may not be needed on newer versions of the MSP430
 *
 * \param pRtcData
 */
void halRtcSet(tRtcHostMsgPayload* pRtcData);

/*! Get the current structure containing the real time clock parameters.
 *
 * \param pRtcData
 *
 */
void halRtcGet(tRtcHostMsgPayload* pRtcData);


int SetRTCYEAR(int year);
int SetRTCMON(int month);
int SetRTCDAY(int day);
int SetRTCDOW(int dow);
int SetRTCHOUR(int hour);
int SetRTCMIN(int min);
int SetRTCSEC(int sec);

int GetRTCTIM0(void);
int GetRTCTIM1(void);
int GetRTCDATE(void);
int GetRTCYEAR(void);

int GetRTCMON(void);
int GetRTCDOW(void);
int GetRTCDAY(void);
int GetRTCHOUR(void);
int GetRTCMIN(void);
int GetRTCSEC(void);

#endif /* HAL_RTC_H */
