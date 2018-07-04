#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "Defines.h"
#include "Messages.h"
#include "MessageQueues.h"
#include "LcdDisplay.h"
#include "LcdBuffer.h"
#include "BufferPool.h"
#include "DebugUart.h"

/* 96 * 12 */
#define BYTES_PER_SCREEN    ( (unsigned int)1152 )
#define BYTES_PER_LINE      ( 12 )

static unsigned char IdleActiveBuffer[BYTES_PER_SCREEN];
static unsigned char IdleDrawBuffer[BYTES_PER_SCREEN];
static unsigned char ApplicationActiveBuffer[BYTES_PER_SCREEN];
static unsigned char ApplicationDrawBuffer[BYTES_PER_SCREEN];
static unsigned char NotificationActiveBuffer[BYTES_PER_SCREEN];
static unsigned char NotificationDrawBuffer[BYTES_PER_SCREEN];
static unsigned char ScrollActiveBuffer[BYTES_PER_SCREEN / 2];
static unsigned char ScrollDrawBuffer[BYTES_PER_SCREEN / 2];

static unsigned char *pIdleActiveBuffer;
static unsigned char *pIdleDrawBuffer;
static unsigned char *pApplicationActiveBuffer;
static unsigned char *pApplicationDrawBuffer;
static unsigned char *pNotificationActiveBuffer;
static unsigned char *pNotificationDrawBuffer;
static unsigned char *pScrollActiveBuffer;
static unsigned char *pScrollDrawBuffer;
/******************************************************************************/
unsigned char GetStartingRow(unsigned char MsgOptions);
unsigned char *GetActiveBufferStartAddress(unsigned char MsgOptions);
unsigned char *GetDrawBufferStartAddress(unsigned char MsgOptions);

static tHostMsg* pWriteLcdMsg;
/******************************************************************************/
void InitialiazeFrameBuffer(void)
{
    memset(IdleActiveBuffer,            0, BYTES_PER_SCREEN);
    memset(IdleDrawBuffer,              0, BYTES_PER_SCREEN);
    memset(ApplicationActiveBuffer,     0, BYTES_PER_SCREEN);
    memset(ApplicationDrawBuffer,       0, BYTES_PER_SCREEN);
    memset(NotificationActiveBuffer,    0, BYTES_PER_SCREEN);
    memset(NotificationDrawBuffer,      0, BYTES_PER_SCREEN);
    memset(ScrollActiveBuffer,          0, BYTES_PER_SCREEN / 2);
    memset(ScrollDrawBuffer,            0, BYTES_PER_SCREEN / 2);

    pIdleActiveBuffer           = IdleActiveBuffer;
    pIdleDrawBuffer             = IdleDrawBuffer;
    pApplicationActiveBuffer    = ApplicationActiveBuffer;
    pApplicationDrawBuffer      = ApplicationDrawBuffer;
    pNotificationActiveBuffer   = NotificationActiveBuffer;
    pNotificationDrawBuffer     = NotificationDrawBuffer;
    pScrollActiveBuffer         = ScrollActiveBuffer;
    pScrollDrawBuffer           = ScrollDrawBuffer;
}

/* Get the start address of the active buffer in SRAM */
static unsigned char * GetActiveBufferStartAddress(unsigned char MsgOptions)
{
    unsigned char BufferSelect = MsgOptions & BUFFER_SELECT_MASK;
    unsigned char *pBuffer;

    switch (BufferSelect)
    {
    case IDLE_BUFFER_SELECT:
        pBuffer = pIdleActiveBuffer;
        break;
    case APPLICATION_BUFFER_SELECT:
        pBuffer = pApplicationActiveBuffer;
        break;
    case NOTIFICATION_BUFFER_SELECT:
        pBuffer = pNotificationActiveBuffer;
        break;
    case SCROLL_BUFFER_SELECT:
        pBuffer = pScrollActiveBuffer;
        break;
    default:
        break;
    }

    return pBuffer;
}

/* Get the start address of the Draw buffer in SRAM */
static unsigned char * GetDrawBufferStartAddress(unsigned char MsgOptions)
{
    unsigned char BufferSelect = MsgOptions & BUFFER_SELECT_MASK;
    unsigned char *pBuffer;

    switch (BufferSelect)
    {
    case IDLE_BUFFER_SELECT:
        pBuffer = pIdleDrawBuffer;
        break;
    case APPLICATION_BUFFER_SELECT:
        pBuffer = pApplicationDrawBuffer;
        break;
    case NOTIFICATION_BUFFER_SELECT:
        pBuffer = pNotificationDrawBuffer;
        break;
    case SCROLL_BUFFER_SELECT:
        pBuffer = pScrollDrawBuffer;
        break;
    default:
        break;
    }

    return pBuffer;
}

/* Activate the current draw buffer (swap pointers) */
static void ActivateBuffer(tHostMsg* pMsg)
{
    unsigned char BufferSelect = (pMsg->Options) & BUFFER_SELECT_MASK;
    unsigned char *pBufferSwap;
    switch (BufferSelect)
    {
    case 0:
        pBufferSwap = pIdleActiveBuffer;
        pIdleActiveBuffer = pIdleDrawBuffer;
        pIdleDrawBuffer = pBufferSwap;
        break;
    case 1:
        pBufferSwap = pApplicationActiveBuffer;
        pApplicationActiveBuffer = pApplicationDrawBuffer;
        pApplicationDrawBuffer = pBufferSwap;
        break;
    case 2:
        pBufferSwap = pNotificationActiveBuffer;
        pNotificationActiveBuffer = pNotificationDrawBuffer;
        pNotificationDrawBuffer = pBufferSwap;
        break;
    case 3:
        pBufferSwap = pScrollActiveBuffer;
        pScrollActiveBuffer = pScrollDrawBuffer;
        pScrollDrawBuffer = pBufferSwap;
        break;
    default:
        PrintStringAndHex("Invalid Buffer Selected: 0x",BufferSelect);
        break;
    }
}

/* determine if the phone is controlling all of the idle screen */
unsigned char GetStartingRow(unsigned char MsgOptions)
{
  unsigned char StartingRow = 0;

  if (   (MsgOptions & BUFFER_SELECT_MASK) == IDLE_BUFFER_SELECT
      && GetIdleBufferConfiguration() == WATCH_CONTROLS_TOP )
  {
    StartingRow = 30;
  }
  else
  {
    StartingRow = 0;
  }

  return StartingRow;
}

void WriteBufferHandler(tHostMsg* pMsg)
{
    /*
    * save the parameters that are going to get written over
    */
    unsigned char MsgOptions = pMsg->Options;
    unsigned char RowA = pMsg->pPayload[WRITE_BUFFER_ROW_A_INDEX];
    unsigned char RowB = pMsg->pPayload[WRITE_BUFFER_ROW_B_INDEX];
    unsigned char *pBufferStartAddress = GetDrawBufferStartAddress(MsgOptions);

    memcpy(pBufferStartAddress + BYTES_PER_LINE * RowA, pMsg->pPayload + 1, BYTES_PER_LINE);

    /* if the bit is one then only draw one line */
    if ( (MsgOptions & WRITE_BUFFER_ONE_LINE_MASK) == 0 )
    {
        memcpy(pBufferStartAddress + BYTES_PER_LINE * RowB, pMsg->pPayload + 14, BYTES_PER_LINE);
    }
}

unsigned char UpdateDisplayHandler(tHostMsg* pMsg)
{
    unsigned char LcdRow;
    unsigned char Options = pMsg->Options;
    unsigned char * pBufferAddress;
    unsigned char * pDrawBufferAddress;
    unsigned char i;

    if ( (Options & DRAW_BUFFER_ACTIVATION_MASK) == ACTIVATE_DRAW_BUFFER )
    {
        ActivateBuffer(pMsg);
    }

    /* premature exit */
    if ( (Options & BUFFER_SELECT_MASK) == IDLE_BUFFER_SELECT
      && QueryIdlePageNormal() == 0 )
    {
        return FREE_BUFFER;
    }

    pBufferAddress = GetActiveBufferStartAddress(Options);
    pDrawBufferAddress = GetDrawBufferStartAddress(Options);

    /* if it is the idle buffer; determine starting line */
    LcdRow = GetStartingRow(Options);

    for ( ; LcdRow < 96; LcdRow++ ) {
        tLcdMessage* pLcdMessage;
        BPL_AllocMessageBuffer(&pWriteLcdMsg);
        pLcdMessage = (tLcdMessage*)pWriteLcdMsg;

        /* if there was more ram then it would be better to do a screen copy  */
        if ( (Options & UPDATE_COPY_MASK ) == COPY_ACTIVE_TO_DRAW_DURING_UPDATE) {
            for (i = 0; i < BYTES_PER_LINE; i++) {
                pDrawBufferAddress[LcdRow * BYTES_PER_LINE + i] = pBufferAddress[LcdRow * BYTES_PER_LINE + i];
            }
        }

        /*! todo change to query full */
        while( uxQueueMessagesWaiting(QueueHandles[LCD_TASK_QINDEX]) > 9 );

        /* now format the message that is going to go to the LCD task */
        pLcdMessage->Type = WriteLcd;
        pLcdMessage->Options = NO_MSG_OPTIONS;
        pLcdMessage->RowNumber = LcdRow;

        for (i = 0; i < BYTES_PER_LINE; i++) {
            pLcdMessage->pLine[i] = BitRev8(pBufferAddress[LcdRow * BYTES_PER_LINE + i]);
        }

        RouteMsg(&pWriteLcdMsg);
    }

    /*! wait until everything has been written to the LCD */
    while( uxQueueMessagesWaiting(QueueHandles[LCD_TASK_QINDEX]) > 0 );

    /*
     * send chain mail - now tell the display task that the operation
     * has completed
     */
    pMsg->Type = ChangeModeMsg;
    pMsg->Options = Options;
    RouteMsg(&pMsg);

    return DO_NOT_FREE_BUFFER;
}
