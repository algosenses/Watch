//==============================================================================
//  Copyright 2011 Meta Watch Ltd. - http://www.MetaWatch.org/
// 
//  Licensed under the Meta Watch License, Version 1.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.MetaWatch.org/licenses/license-1.0.html
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//==============================================================================

/******************************************************************************/
/*! \file Display.h
 *
 * Handle functions common to both types of display (OLED and LCD).
 */
/******************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H


/*! Copies the local bluetooth address string (from SerialProfile) 
 * so that the display task can put it on the screen
 */
void SetLocalBluetoothAddressString(unsigned char* pData);

/*! Copies the remote bluetooth address string so that the display task can put 
 * it on the screen
 */
void SetRemoteBluetoothAddressString(unsigned char* pData);

/*! Returns a pointer to the Local Bluetooth Address 
 * 
 */
unsigned char* GetLocalBluetoothAddressString(void);

/*! Returns a pointer to the Remote Bluetooth Address 
 *
 */
unsigned char* GetRemoteBluetoothAddressString(void);

/*! Find out what display mode the Display Task is in
 *
 *\return Returns a display mode as defined in messages.h
 */
unsigned char QueryDisplayMode(void);

/*! Reads the hardware revision register in the MSP430
 *
 * \return Character representation of revision (Example: 'E')
 */
unsigned char GetHardwareRevision(void);

/*! Query the connection state of the bluetooth serial port
 * 
 * \return String representation of the current state of serial port.
 */
unsigned char * QueryConnectionStateAndGetString(void);


/*! This is called by the stack to set the FirstContact variable.  This 
 * variable is used to determine if the phone has connected for the first time
 */
void SetFirstContact(void);

/*! Clear the first contact flag */
void ClearFirstContact(void);


/*! Query the FirstContact Flag
 *
 * \return 1 = phone has been connected, 0 otherwise
 */
unsigned char QueryFirstContact(void);

/*! Strings for days of the week in the same order as RTC */
extern const unsigned char DaysOfTheWeek[][7];

/*! Strings for months of the year in the same format as RTC */
extern const unsigned char MonthsOfYear[][13];

/******************************************************************************/

/*! Twelve hour mode for TimeFormat */
#define TWELVE_HOUR      ( 0 )
/*! 24 hour mode for TimeFormat */
#define TWENTY_FOUR_HOUR ( 1 )

/*! Display the month first */
#define MONTH_FIRST ( 0 )

/*! Display the day before the month */
#define DAY_FIRST   ( 1 )

/*! Initaliaze the non-volatile item that holds the time format (which is 12 or
 * 24 hour )
*/
void InitializeTimeFormat(void);

/*! Initaliaze the non-volatile item that holds the date format (which is day
 * or month first )
*/
void InitializeDateFormat(void);

/*! \return Time Format TWELVE_HOUR = 0, TWENTY_FOUR_HOUR = 1 */
unsigned char GetTimeFormat(void);

/* swaps the time format */
void ToggleTimeFormat(void);

/* Saves the time format to NV */
void SaveTimeFormat(void);

/*! \return date format MONTH_FIRST = 0, DAY_FIRST = 1 */
unsigned char GetDateFormat(void);

/* Swaps the date format */
void ToggleDateFormat(void);

/* Saves the date format to NV */
void SaveDateFormat(void);

/******************************************************************************/

/*! \return 1 if the link alarm is enabled, 0 if it is not */
unsigned char QueryLinkAlarmEnable(void);

/*! Toggle the state of the link Alarm control bit */
void ToggleLinkAlarmEnable(void);

/*! Initialize the nv item that determines if vibration should occur when the
 * link is lost 
*/
void InitializeLinkAlarmEnable(void);

/*! Save the link alarm control bit into nval */
void SaveLinkAlarmEnable(void);

/*! Send a vibration message to the vibration state machine */
void GenerateLinkAlarm(void);


/******************************************************************************/

/*! Initialize the nv items that hold the application and notification mode timeouts */
void InitializeModeTimeouts(void);

/*! \return Application mode timeout in seconds */
unsigned int QueryApplicationModeTimeout(void);

/*! \return Notification mode timeout in seconds */
unsigned int QueryNotificationModeTimeout(void);

/******************************************************************************/

#define SNIFF_DEBUG_DEFAULT      ( 0 )
#define BATTERY_DEBUG_DEFAULT    ( 0 ) 
#define CONNECTION_DEBUG_DEFAULT ( 0 ) 

/*! Initialize flags stored in nval used for debugging (controlling print statements */
void InitializeDebugFlags(void);

/* \return 1 when sniff debug messages should be printed */
unsigned char QuerySniffDebug(void);

/* \return 1 when battery debug messages should be printed */
unsigned char QueryBatteryDebug(void);

/* \return 1 when connection debug messages should be printed */
unsigned char QueryConnectionDebug(void);

#endif /* DISPLAY_H */
