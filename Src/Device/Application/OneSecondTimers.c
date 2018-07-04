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
/*! \file OneSecondTimers.c
*
*/
/******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "Messages.h"
#include "MessageQueues.h"
#include "BufferPool.h"
#include "DebugUart.h"
#include "Utilities.h"
#include "CommandTask.h"
#include "OneSecondTimers.h"

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "interrupt.h"

#define ENTER_CRITICAL_REGION_QUICK()   IntMasterDisable()
#define LEAVE_CRITICAL_REGION_QUICK()   IntMasterEnable()

/*! One Second Timer Structure */
typedef struct
{
  unsigned int Timeout;
  unsigned int DownCounter;
  unsigned char Allocated;
  unsigned char Running;
  unsigned char Expired;
  unsigned char RepeatCount;
  eMessageType CallbackMsgType;
  unsigned char CallbackMsgOptions;

} tOneSecondTimer;


// using last location for debug
static tOneSecondTimer OneSecondTimers[TOTAL_ONE_SECOND_TIMERS];

void InitializeOneSecondTimers(void)
{
  /* clear information for all of the timers */
  unsigned char i;
  for (i = 0; i < TOTAL_ONE_SECOND_TIMERS; i++ )
  {
    OneSecondTimers[i].Timeout = 0;
    OneSecondTimers[i].DownCounter = 0;
    OneSecondTimers[i].Allocated = 0;
    OneSecondTimers[i].Running = 0;
    OneSecondTimers[i].Expired = 0;
    OneSecondTimers[i].RepeatCount = 0;
    OneSecondTimers[i].CallbackMsgType = InvalidMessage;
    OneSecondTimers[i].CallbackMsgOptions = 0;

  }

}


tTimerId AllocateOneSecondTimer(void)
{
  signed char result = -1;
  unsigned char i;

  ENTER_CRITICAL_REGION_QUICK();
  for(i = 0; i < TOTAL_ONE_SECOND_TIMERS; i++)
  {
    if ( OneSecondTimers[i].Allocated == 0 )
    {
      OneSecondTimers[i].Allocated = 1;
      result = i;
      break;
    }

  }

  LEAVE_CRITICAL_REGION_QUICK();

  if ( result < 0 )
  {
    PrintString("Unable to allocate Timer\r\n");
  }

  return result;
}


signed char DeallocateOneSecondTimer(tTimerId TimerId)
{
  signed char result = -1;

  if ( TimerId < 0 )
  {
    PrintString("Invalid Timer Id\r\n");
  }

  ENTER_CRITICAL_REGION_QUICK();

  if ( OneSecondTimers[TimerId].Allocated == 1 )
  {
    OneSecondTimers[TimerId].Allocated = 0;
    OneSecondTimers[TimerId].Running = 0;
    OneSecondTimers[TimerId].Expired = 0;
    result = TimerId;
  }

  LEAVE_CRITICAL_REGION_QUICK();

  if ( result < 0 )
  {
    PrintString("Unable to deallocate timer!\r\n");
  }

  return result;
}



void StartOneSecondTimer(tTimerId TimerId)
{
  if (  OneSecondTimers[TimerId].Allocated == 0 ||
        OneSecondTimers[TimerId].CallbackMsgType == InvalidMessage )
  {
    PrintString("Cannot start timer with invalid parameters\r\n");
    return;
  }

  ENTER_CRITICAL_REGION_QUICK();

  OneSecondTimers[TimerId].Running = 1;
  OneSecondTimers[TimerId].Expired = 0;
  OneSecondTimers[TimerId].DownCounter = OneSecondTimers[TimerId].Timeout;

  LEAVE_CRITICAL_REGION_QUICK();
}

void StopOneSecondTimer(tTimerId TimerId)
{
  ENTER_CRITICAL_REGION_QUICK();

  OneSecondTimers[TimerId].Running = 0;
  OneSecondTimers[TimerId].Expired = 0;

  LEAVE_CRITICAL_REGION_QUICK();
}

/* setup a timer so the Restart timer function can be used */
void SetupOneSecondTimer(tTimerId TimerId,
                         unsigned int Timeout,
                         unsigned char RepeatCount,
                         eMessageType CallbackMsgType,
                         unsigned char MsgOptions)
{

  if (   OneSecondTimers[TimerId].Allocated == 0
      || TimerId < 0 )
  {
    PrintString("Timer not Allocated\r\n");
    return;
  }

  ENTER_CRITICAL_REGION_QUICK();

  OneSecondTimers[TimerId].RepeatCount = RepeatCount;
  OneSecondTimers[TimerId].Timeout = Timeout;
  OneSecondTimers[TimerId].CallbackMsgType = CallbackMsgType;
  OneSecondTimers[TimerId].CallbackMsgOptions = MsgOptions;

  LEAVE_CRITICAL_REGION_QUICK();
}

#if 0
void ChangeOneSecondTimerTimeout(tTimerId TimerId,
                                 unsigned int Timeout)
{
  OneSecondTimers[TimerId].Timeout = Timeout;
}

void ChangeOneSecondTimerMsgOptions(tTimerId TimerId,
                                    unsigned char MsgOptions)
{
  OneSecondTimers[TimerId].CallbackMsgOptions = MsgOptions;
}

void ChangeOneSecondTimerMsgCallback(tTimerId TimerId,
                                     eMessageType CallbackMsgType;
                                     unsigned char MsgOptions)
{
  OneSecondTimers[TimerId].CallbackMsgType = CallbackMsgType;
  OneSecondTimers[TimerId].CallbackMsgOptions = MsgOptions;
}
#endif

/* this should be as fast as possible because it happens in interrupt context
 * and it also often occurs when the part is sleeping
 */
unsigned char OneSecondTimerHandlerIsr(void)
{
  unsigned char ExitLpm = 0;
  unsigned char i;
  for (i = 0; i < TOTAL_ONE_SECOND_TIMERS; i++ )
  {
    if ( OneSecondTimers[i].Running == 1 )
    {

      /* decrement the counter first */
      if ( OneSecondTimers[i].DownCounter > 0 )
      {
        OneSecondTimers[i].DownCounter--;
      }

      /* has the counter reached 0 ?*/
      if ( OneSecondTimers[i].DownCounter == 0 )
      {
        /* should the counter be reloaded or stopped */
        if ( OneSecondTimers[i].RepeatCount == 0xFF )
        {
          OneSecondTimers[i].DownCounter = OneSecondTimers[i].Timeout;
        }
        else if ( OneSecondTimers[i].RepeatCount > 0 )
        {
          OneSecondTimers[i].DownCounter = OneSecondTimers[i].Timeout;
          OneSecondTimers[i].RepeatCount--;
        }
        else
        {
          OneSecondTimers[i].Running = 0;
        }

        OneSecondTimers[i].Expired = 1;
        ExitLpm = 1;
        /*! todo - pass the index of the expired timer and then
         * make expired handler more efficient (another reason for a new type
         * of queue )
         */
        RouteCommandFromIsr(ExpiredTimerCmd);

      }
    }
  }

  return ExitLpm;

}

static tHostMsg* pOneSecondMsg;

void ExpiredOneSecondTimerHandler(void)
{
  tOneSecondTimer CurrentTimer;
  unsigned char i;
  for (i = 0; i < TOTAL_ONE_SECOND_TIMERS; i++ )
  {
    ENTER_CRITICAL_REGION_QUICK();
    CurrentTimer = OneSecondTimers[i];
    OneSecondTimers[i].Expired = 0;
    LEAVE_CRITICAL_REGION_QUICK();

    if ( CurrentTimer.Expired == 1 )
    {
      /* send a callback message */
      BPL_AllocMessageBuffer(&pOneSecondMsg);
      pOneSecondMsg->Type = CurrentTimer.CallbackMsgType;
      pOneSecondMsg->Options = CurrentTimer.CallbackMsgOptions;
      RouteMsg(&pOneSecondMsg);

    }
  }

}


