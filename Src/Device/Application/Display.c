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
/*! \file Display.c
 *
 * Handle functions common to both types of display
 */
/******************************************************************************/

#include "FreeRTOS.h"
#include "queue.h"

#include "Messages.h"
#include "BufferPool.h"
#include "MessageQueues.h"
#include "OneSecondTimers.h"
#include "Display.h"
#include "SerialProfile.h"

/* these have the null character added at the end */
unsigned char pLocalBluetoothAddressString[] = "000000000000";
unsigned char pRemoteBluetoothAddressString[] = "000000000000";

/* LocalBluetoothAddress == watch */
void SetLocalBluetoothAddressString(unsigned char* pData)
{
  unsigned char i = 0;

  while ( pLocalBluetoothAddressString[i] != 0 && pData[i] != 0 )
  {
    pLocalBluetoothAddressString[i] = pData[i];
    i++;
  }

}


/* RemoteBluetoothAddress == phone */
void SetRemoteBluetoothAddressString(unsigned char* pData)
{
  unsigned char i = 0;

  while ( pRemoteBluetoothAddressString[i] != 0 && pData[i] != 0 )
  {
    pRemoteBluetoothAddressString[i] = pData[i];
    i++;
  }

}

unsigned char* GetLocalBluetoothAddressString(void)
{
  return pLocalBluetoothAddressString;
}

unsigned char* GetRemoteBluetoothAddressString(void)
{
  return pRemoteBluetoothAddressString;
}

/******************************************************************************/

#define HARDWARE_REVISION_ADDRESS (0x1a07)

unsigned char GetHardwareRevision(void)
{
  unsigned char *pDeviceType = (unsigned char *)(unsigned char *)HARDWARE_REVISION_ADDRESS;
  return pDeviceType[0]+'1';
}


/******************************************************************************/

unsigned char * QueryConnectionStateAndGetString(void)
{
  etConnectionState cs = QueryConnectionState();
  unsigned char * pString;

  /* Initializing is the longest word that can fit on the LCD */
  switch (cs)
  {
  case Initializing:       pString = "Initializing"; break;
  case ServerFailure:      pString = "ServerFail";   break;
  case RadioOn:            pString = "RadioOn";      break;
  case Paired:             pString = "Paired";       break;
  case Connected:          pString = "Connected";    break;
  case RadioOff:           pString = "RadioOff";     break;
  case RadioOffLowBattery: pString = "RadioOff";     break;
  case ShippingMode:       pString = "ShippingMode"; break;
  default:                 pString = "Unknown";      break;
  }

  return pString;
}

/******************************************************************************/

unsigned char FirstContact;

void SetFirstContact(void)
{
  FirstContact = 1;
}

void ClearFirstContact(void)
{
  FirstContact = 0;
}

unsigned char QueryFirstContact(void)
{
//  return FirstContact;
    return 1;
}

/******************************************************************************/

/*
 * these are setup to match RTC
 * days of week are 0-6 and months are 1-12
 */
const unsigned char DaysOfTheWeek[][7] =
{
  "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

const unsigned char MonthsOfYear[][13] =
{
  "???","Jan","Feb","Mar","Apr","May","June",
  "July","Aug","Sep","Oct","Nov","Dec"
};

/******************************************************************************/

static unsigned char nvTimeFormat;
static unsigned char nvDateFormat;

void InitializeTimeFormat(void)
{
    nvTimeFormat = TWENTY_FOUR_HOUR;
}

unsigned char GetTimeFormat(void)
{
    return nvTimeFormat;
}

void ToggleTimeFormat(void)
{
  if ( nvTimeFormat == 1 )
  {
	  nvTimeFormat = 0;
  }
  else
  {
	  nvTimeFormat = 1;
  }
}

void SaveTimeFormat(void)
{

}

void InitializeDateFormat(void)
{
	nvDateFormat = DAY_FIRST;

}

unsigned char GetDateFormat(void)
{
  return nvDateFormat;
}

void ToggleDateFormat(void)
{
  if ( nvDateFormat == DAY_FIRST )
  {
	  nvDateFormat = MONTH_FIRST;
  }
  else
  {
	  nvDateFormat = DAY_FIRST;
  }
}

void SaveDateFormat(void)
{
}



/******************************************************************************/



/******************************************************************************/

static unsigned int nvApplicationModeTimeout;
static unsigned int nvNotificationModeTimeout;

void InitializeModeTimeouts(void)
{
  nvApplicationModeTimeout  = ONE_SECOND*60*10;
  nvNotificationModeTimeout = ONE_SECOND*30;
}

unsigned int QueryApplicationModeTimeout(void)
{
  return nvApplicationModeTimeout;
}

unsigned int QueryNotificationModeTimeout(void)
{
  return nvNotificationModeTimeout;
}

/******************************************************************************/

static unsigned char nvSniffDebug;
static unsigned char nvBatteryDebug;
static unsigned char nvConnectionDebug;

void InitializeDebugFlags(void)
{
  nvSniffDebug = SNIFF_DEBUG_DEFAULT;
  nvBatteryDebug = BATTERY_DEBUG_DEFAULT;
  nvConnectionDebug = CONNECTION_DEBUG_DEFAULT;
}

unsigned char QuerySniffDebug(void)
{
  return nvSniffDebug;
}

unsigned char QueryBatteryDebug(void)
{
  return nvBatteryDebug;
}

unsigned char QueryConnectionDebug(void)
{
  return nvConnectionDebug;
}
