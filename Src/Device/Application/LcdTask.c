#include "rit128x96x4.h"

#include "FreeRTOS.h"
#include "Task.h"
#include "Queue.h"

#include "Messages.h"
#include "MessageQueues.h"
#include "BufferPool.h"
#include "LcdDisplay.h"
#include "LcdTask.h"
#include "DebugUart.h"

#define LCD_TASK_MSG_QUEUE_LEN   12
#define LCD_TASK_STACK_DEPTH     (configMINIMAL_STACK_SIZE + 120)
#define LCD_TASK_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)

/******************************************************************************/
static void LcdTask(void *pvParameters);
static void LcdTaskMessageHandler(tHostMsg* pMsg);

static tHostMsg* pLcdTaskMsg;
static void WriteLcdHandler(tHostMsg* pMsg);
static void ClearLcdHandler(void);
static void UpdateMyDisplayHandler(tHostMsg* pMsg);

xTaskHandle LcdTaskHandle;
/******************************************************************************/

static void LcdPeripheralInit(void)
{
    RIT128x96x4Init(1000000);
}

void InitializeLcdTask(void)
{
    LcdPeripheralInit();

    // This is a Rx message queue
    QueueHandles[LCD_TASK_QINDEX] =
                xQueueCreate( LCD_TASK_MSG_QUEUE_LEN, MESSAGE_QUEUE_ITEM_SIZE );

    // prams are: task function, task name, stack len , task params, priority, task handle
    xTaskCreate(LcdTask,
              "LCD_DRIVER",
              LCD_TASK_STACK_DEPTH,
              NULL,
              LCD_TASK_TASK_PRIORITY,
              &LcdTaskHandle);
}

/*! The LCD task runs forever.  It writes data to the LCD based on the
 * commands it receives in its queue.
 */
static void LcdTask(void *pvParameters)
{
    if ( QueueHandles[LCD_TASK_QINDEX] == 0 )
    {
        PrintString("Lcd Task Queue not created!\r\n");
    }

    /* the first task is to clear the LCD */
    BPL_AllocMessageBuffer(&pLcdTaskMsg);
    pLcdTaskMsg->Type = ClearLcd;
    RouteMsg(&pLcdTaskMsg);

    while (1) {
        if( pdTRUE == xQueueReceive(QueueHandles[LCD_TASK_QINDEX],
                                &pLcdTaskMsg, portMAX_DELAY) ) {
            LcdTaskMessageHandler(pLcdTaskMsg);

            BPL_FreeMessageBuffer(&pLcdTaskMsg);
        }
    }
}

/*! Process the messages routed to the LCD driver Task */
static void LcdTaskMessageHandler(tHostMsg* pMsg)
{
  unsigned char Type = pMsg->Type;

  switch(Type)
  {

  case WriteLcd:
    WriteLcdHandler(pMsg);
    break;

  case ClearLcd:
    ClearLcdHandler();
    break;

  case UpdateMyDisplayLcd:
    UpdateMyDisplayHandler(pMsg);
    break;

  default:
    PrintStringAndHex("<<Unhandled Message>> in Lcd Task: Type 0x", Type);
    break;
  }

}

static void UpdateMyDisplayHandler(tHostMsg* pMsg)
{
    tUpdateMyDisplayMsg* pUpdateMyDisplayMessage = (tUpdateMyDisplayMsg*)pMsg;

    WriteLineToLcd(pUpdateMyDisplayMessage->pMyDisplay,
                   0,
                   pUpdateMyDisplayMessage->TotalLines);
}

/*! Writes a single line to the LCD */
static void WriteLcdHandler(tHostMsg* pMsg)
{
    tLcdMessage* pLcdMessage = (tLcdMessage*)pMsg;
    WriteLineToLcd(pLcdMessage->pLine, pLcdMessage->RowNumber, 1);
}

static void ClearLcdHandler(void)
{
    RIT128x96x4Clear();
}
