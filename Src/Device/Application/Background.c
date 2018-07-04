#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Messages.h"
#include "BufferPool.h"
#include "MessageQueues.h"

#include "Background.h"
#include "Utilities.h"
#include "OneSecondTimers.h"
#include "DebugUart.h"
#include "Statistics.h"

#include "hal_rtc.h"

static void BackgroundTask(void *pvParameters);

static void BackgroundMessageHandler(tHostMsg* pMsg);

#define BACKGROUND_MSG_QUEUE_LEN   8
#define BACKGROUND_STACK_DEPTH	   (configMINIMAL_STACK_SIZE + 50)
#define BACKGROUND_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)

xTaskHandle xBkgTaskHandle;

static tHostMsg* pBackgroundMsg;

void SetVibrateModeHandler(tHostMsg* pMsg);
/******************************************************************************/

/*! Does the initialization and allocates the resources for the background task
 *
 */
void InitializeBackgroundTask( void )
{
  // This is a Rx message queue, messages come from Serial IO or button presses
  QueueHandles[BACKGROUND_QINDEX] =
    xQueueCreate( BACKGROUND_MSG_QUEUE_LEN, MESSAGE_QUEUE_ITEM_SIZE );

  // prams are: task function, task name, stack len , task params, priority, task handle
  xTaskCreate(BackgroundTask,
              "BACKGROUND",
              BACKGROUND_STACK_DEPTH,
              NULL,
              BACKGROUND_TASK_PRIORITY,
              &xBkgTaskHandle);

}

/*! Function to implement the BackgroundTask loop
 *
 * \param pvParameters not used
 *
 */
static void BackgroundTask(void *pvParameters)
{
  if ( QueueHandles[BACKGROUND_QINDEX] == 0 )
  {
    PrintString("Background Queue not created!\r\n");
  }

  for(;;)
  {
    if( pdTRUE == xQueueReceive(QueueHandles[BACKGROUND_QINDEX],
                                &pBackgroundMsg, portMAX_DELAY ) )
    {
      BackgroundMessageHandler(pBackgroundMsg);

      BPL_FreeMessageBuffer(&pBackgroundMsg);

    }

  }

}

/*! Handle the messages routed to the background task */
static void BackgroundMessageHandler(tHostMsg* pMsg)
{
  tHostMsg* pOutgoingMsg;

  eMessageType Type = (eMessageType)pMsg->Type;

  switch(Type)
  {
  case GetDeviceType:
    BPL_AllocMessageBuffer(&pOutgoingMsg);

    pOutgoingMsg->pPayload[0] = 2; /* DIGITAL_BOARD_TYPE */

    UTL_BuildHstMsg(pOutgoingMsg, GetDeviceTypeResponse, NO_MSG_OPTIONS,
                    pOutgoingMsg->pPayload, sizeof(unsigned char));

    RouteMsg(&pOutgoingMsg);
    break;

  case SetVibrateMode:
    SetVibrateModeHandler(pMsg);
    break;

  case SetRealTimeClock:
    halRtcSet((tRtcHostMsgPayload*)pMsg->pPayload);

    BPL_AllocMessageBuffer(&pOutgoingMsg);
    pOutgoingMsg->Type = IdleUpdate;
    pOutgoingMsg->Options = NO_MSG_OPTIONS;
    RouteMsg(&pOutgoingMsg);

    break;

  default:
    PrintStringAndHex("<<Unhandled Message>> in Background Task: Type 0x", Type);
    break;
  }

}

/* Handle the message from the host that starts a vibration event */
void SetVibrateModeHandler(tHostMsg* pMsg)
{
}
