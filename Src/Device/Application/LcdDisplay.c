#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DebugUart.h"
#include "Messages.h"
#include "MessageQueues.h"
#include "OneSecondTimers.h"
#include "LcdBuffer.h"
#include "LcdTask.h"
#include "LcdDisplay.h"
#include "BufferPool.h"
#include "Display.h"
#include "IdlePage.h"
#include "IdlePageMain.h"
#include "IdlePageGameOfLife.h"
#include "IdlePageQrCode.h"
#include "FrameBuffer.h"
#include "Utilities.h"
#include "hal_lcd.h"

#define DISPLAY_TASK_QUEUE_LENGTH   8
#define DISPLAY_TASK_STACK_DEPTH	(configMINIMAL_STACK_SIZE + 90)
#define DISPLAY_TASK_PRIORITY       (tskIDLE_PRIORITY + 1)

extern const unsigned char pMetaWatchSplash[NUM_LCD_ROWS*NUM_LCD_COL_BYTES];

xTaskHandle DisplayHandle;

static unsigned char DisplayQueueMessageHandler(tHostMsg* pMsg);
static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg);
static void ChangeModeHandler(tHostMsg* pMsg);
void SendMyBufferToLcd(unsigned char TotalRows);

static tHostMsg* pDisplayMsg;
static tTimerId IdleModeTimerId;
static tTimerId ApplicationModeTimerId;
static tTimerId NotificationModeTimerId;

/* the internal buffer */
#define STARTING_ROW                  ( 0 )
#define WATCH_DRAWN_IDLE_BUFFER_ROWS  ( 30 )
#define PHONE_IDLE_BUFFER_ROWS        ( 66 )

tLcdLine pMyBuffer[NUM_LCD_ROWS];

static unsigned char nvIdleBufferConfig;
static unsigned char nvIdleBufferInvert;

static void InitialiazeIdleBufferConfig(void);
static void InitializeIdleBufferInvert(void);

void SaveIdleBufferInvert(void);


/******************************************************************************/

static unsigned char LastMode = IDLE_MODE;
static unsigned char CurrentMode = IDLE_MODE;

static unsigned char AllowConnectionStateChangeToUpdateScreen;

/*! Display the startup image or Splash Screen */
static void DisplayStartupScreen(void)
{
    CopyRowsIntoMyBuffer(pMetaWatchSplash, STARTING_ROW, NUM_LCD_ROWS);
    PrepareMyBufferForLcd(STARTING_ROW, NUM_LCD_ROWS);
    SendMyBufferToLcd(NUM_LCD_ROWS);
}

/*! Allocate ids and setup timers for the display modes */
static void AllocateDisplayTimers(void)
{
  IdleModeTimerId = AllocateOneSecondTimer();

  ApplicationModeTimerId = AllocateOneSecondTimer();

  NotificationModeTimerId = AllocateOneSecondTimer();

}

static void SetupSplashScreenTimeout(void)
{
  SetupOneSecondTimer(IdleModeTimerId,
                      ONE_SECOND*1,
                      NO_REPEAT,
                      SplashTimeoutMsg,
                      NO_MSG_OPTIONS);

  StartOneSecondTimer(IdleModeTimerId);

  AllowConnectionStateChangeToUpdateScreen = 0;
}

void StopAllDisplayTimers(void)
{
  StopOneSecondTimer(IdleModeTimerId);
  StopOneSecondTimer(ApplicationModeTimerId);
  StopOneSecondTimer(NotificationModeTimerId);

}

/*! LCD Task Main Loop
 *
 * \param pvParameters
 *
 */
static void DisplayTask(void *pvParameters)
{
    unsigned char FreeIt;

    if ( QueueHandles[DISPLAY_QINDEX] == 0 )
    {
        PrintString("Display Queue not created!\r\n");
    }

//    DisplayStartupScreen();

    InitialiazeIdleBufferConfig();
    InitializeIdleBufferInvert();
    InitializeDisplaySeconds();
    InitializeTimeFormat();
    InitializeDateFormat();
    AllocateDisplayTimers();

    InitIdlePage(IdleModeTimerId, pMyBuffer);

    SetupSplashScreenTimeout();

    InitialiazeFrameBuffer();

    while (1) {
        if ( pdTRUE == xQueueReceive(QueueHandles[DISPLAY_QINDEX],
                                &pDisplayMsg, portMAX_DELAY) ) {

            FreeIt = DisplayQueueMessageHandler(pDisplayMsg);

            if (FreeIt == FREE_BUFFER) {
                BPL_FreeMessageBuffer(&pDisplayMsg);
            }
        }
    }
}

/*! Handle the messages routed to the display queue */
static unsigned char DisplayQueueMessageHandler(tHostMsg* pMsg)
{
    unsigned char FreeMessageBuffer = FREE_BUFFER;
    unsigned char Type = pMsg->Type;

    switch (Type) {
    case IdleUpdate:
        IdlePageHandler(&IdlePageMain);
        break;

    case ChangeModeMsg:
        ChangeModeHandler(pMsg);
        break;

    case BarCode:
        IdlePageHandler(&IdlePageQrCode);
        IdlePageQrCodeButtonHandler(pMsg->Options);
        break;

    case ConfigureMode:
        break;

    case ConfigureIdleBufferSize:
        ConfigureIdleBuferSizeHandler(pMsg);
        break;

    case SplashTimeoutMsg:
        AllowConnectionStateChangeToUpdateScreen = 1;
	    IdlePageHandler(&IdlePageMain);
        break;

    case WriteBuffer:
        WriteBufferHandler(pMsg);
        break;

    case UpdateDisplay:
        FreeMessageBuffer = UpdateDisplayHandler(pMsg);
        break;

    default:
        PrintStringAndHex("<<Unhandled Message>> in Lcd Display Task: Type 0x", Type);
        break;
    }

    return FreeMessageBuffer;
}

/******************************************************************************/

/*! Initialize the LCD display task
 *
 * Initializes the display driver, clears the display buffer and starts the
 * display task
 *
 * \return none, result is to start the display task
 */
void InitializeDisplayTask(void)
{
    QueueHandles[DISPLAY_QINDEX] =
        xQueueCreate( DISPLAY_TASK_QUEUE_LENGTH, MESSAGE_QUEUE_ITEM_SIZE  );

    // task function, task name, stack len , task params, priority, task handle
    xTaskCreate(DisplayTask,
              "DISPLAY",
              DISPLAY_TASK_STACK_DEPTH,
              NULL,
              DISPLAY_TASK_PRIORITY,
              &DisplayHandle);
}

unsigned char QueryDisplayMode(void)
{
  return CurrentMode;
}

static void ChangeModeHandler(tHostMsg* pMsg)
{
    LastMode = CurrentMode;
    CurrentMode = (pMsg->Options & MODE_MASK);

    switch ( CurrentMode ) {
    case IDLE_MODE:
        /* this check is so that the watch apps don't mess up the timer */
        if ( LastMode != CurrentMode ) {
            /* idle update handler will stop all display clocks */
            IdlePageHandler(&IdlePageMain);
            PrintString("Changing mode to Idle\r\n");
        } else {
            PrintString("Already in Idle mode\r\n");
        }
        break;

    case APPLICATION_MODE:
        StopAllDisplayTimers();
        SetupOneSecondTimer(ApplicationModeTimerId,
                            QueryApplicationModeTimeout(),
                            NO_REPEAT,
                            ModeTimeoutMsg,
                            APPLICATION_MODE);
        /* don't start the timer if the timeout == 0
         * this invites things that look like lock ups...
         * it is preferred to make this a large value
         */
        if ( QueryApplicationModeTimeout() ) {
            StartOneSecondTimer(ApplicationModeTimerId);
        }
        PrintString("Changing mode to Application\r\n");
        break;

    case NOTIFICATION_MODE:
        StopAllDisplayTimers();
        SetupOneSecondTimer(NotificationModeTimerId,
                            QueryNotificationModeTimeout(),
                            NO_REPEAT,
                            ModeTimeoutMsg,
                            NOTIFICATION_MODE);
        if ( QueryNotificationModeTimeout() ) {
            StartOneSecondTimer(NotificationModeTimerId);
        }
        PrintString("Changing mode to Notification\r\n");
        break;

    default:
        break;
    }

    /*
     * send a message to the Host indicating buffer update / mode change
     * has completed (don't send message if it is just watch updating time ).
     */
    if ( LastMode != CurrentMode ) {
        tHostMsg* pOutgoingMsg;
        unsigned char data;
        BPL_AllocMessageBuffer(&pOutgoingMsg);
        data = (unsigned char)eScUpdateComplete;
        UTL_BuildHstMsg(pOutgoingMsg, StatusChangeEvent, IDLE_MODE, &data, sizeof(data));
        RouteMsg(&pOutgoingMsg);
    }
}

unsigned char GetIdleBufferConfiguration(void)
{
  return nvIdleBufferConfig;
}

static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg)
{
  nvIdleBufferConfig = pMsg->pPayload[0] & IDLE_BUFFER_CONFIG_MASK;

  if ( nvIdleBufferConfig == WATCH_CONTROLS_TOP )
  {
	  IdlePageHandler(&IdlePageMain);
  }
}

void SendMyBufferToLcd(unsigned char TotalRows)
{
    tHostMsg* pOutgoingMsg;

    BPL_AllocMessageBuffer(&pOutgoingMsg);
    ((tUpdateMyDisplayMsg*)pOutgoingMsg)->Type = UpdateMyDisplayLcd;
    ((tUpdateMyDisplayMsg*)pOutgoingMsg)->TotalLines = TotalRows;
    ((tUpdateMyDisplayMsg*)pOutgoingMsg)->pMyDisplay = (unsigned char*)pMyBuffer;

    RouteMsg(&pOutgoingMsg);
}

static void InitialiazeIdleBufferConfig(void)
{
    nvIdleBufferConfig = WATCH_CONTROLS_TOP;
}

static void InitializeIdleBufferInvert(void)
{
    nvIdleBufferInvert = 0;
}

void ToggleIdleBufferInvert(void)
{
	if ( nvIdleBufferInvert == 1 )
	{
		nvIdleBufferInvert = 0;
	}
	else
	{
		nvIdleBufferInvert = 1;
	}
}

unsigned char QueryInvertDisplay(void)
{
  return nvIdleBufferInvert;
}

void SaveIdleBufferInvert(void)
{
}
