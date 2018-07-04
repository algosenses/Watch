#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Defines.h"
#include "Messages.h"
#include "BufferPool.h"
#include "Buttons.h"
#include "MessageQueues.h"
#include "Messages.h" //IDLE_MODE + msgs
#include "hal_lcd.h"
#include "Icons.h"
#include "Fonts.h"
#include "Display.h"
#include "LcdDisplay.h"
#include "LcdBuffer.h"
#include "IdlePage.h"
#include "SerialProfile.h"
#include "OneSecondTimers.h"

#include "hal_rtc.h"


static int IdleUpdateHandler(struct IdleInfo *Info);
void IdlePageMainConfigButtons(struct IdleInfo *Info);
struct IdlePage IdlePageMain;

void InitIdlePageMain(void)
{
    IdlePageMain.Start = NULL;
    IdlePageMain.Stop = NULL;
    IdlePageMain.Handler = IdleUpdateHandler;
    IdlePageMain.ConfigButtons = IdlePageMainConfigButtons;
}

unsigned char QueryIdlePageNormal(void)
{
  if ( IdlePageCurrent() == &IdlePageMain )
  {
    return 1;
  }
  return 0;
}


const unsigned char Am[10*4];
const unsigned char Pm[10*4];
const unsigned char DaysOfWeek[7][10*4];

static void DisplayAmPm(void);
static void DisplayDayOfWeek(void);
static void DisplayDate(void);

void DisplayDataSeparator(unsigned char RowOffset,
                                 unsigned char ColumnOffset);


unsigned char nvDisplaySeconds = 0;
void InitializeDisplaySeconds(void)
{
    nvDisplaySeconds = 0;
}

int GetDisplaySeconds(void)
{
	return nvDisplaySeconds;
}

void ToggleSecondsHandler(void)
{
	if ( nvDisplaySeconds == 0 )
	{
		nvDisplaySeconds = 1;
	}
	else
	{
		nvDisplaySeconds = 0;
	}
}

void SaveDisplaySeconds(void)
{
}



void DrawIdleScreen(void)
{
  unsigned char msd;
  unsigned char lsd;

  unsigned char Row = 6;
  unsigned char Col = 0;

  int Minutes;

  /* display hour */
  int Hour = GetRTCHOUR();

  /* if required convert to twelve hour format */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    if ( Hour == 0 )
    {
      Hour = 12;
    }
    else if ( Hour > 12 )
    {
      Hour -= 12;
    }
  }

  msd = Hour / 10;
  lsd = Hour % 10;

  /* if first digit is zero then leave location blank */
  if ( msd != 0 )
  {
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
  }
  Col += 1;
  WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;

  /* the colon takes the first 5 bits on the byte*/
  WriteTimeColon(Row,Col,RIGHT_JUSTIFIED);
  Col+=1;

  /* display minutes */
  Minutes = GetRTCMIN();
  msd = Minutes / 10;
  lsd = Minutes % 10;
  WriteTimeDigit(msd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;
  WriteTimeDigit(lsd,Row,Col,LEFT_JUSTIFIED);

  if ( nvDisplaySeconds )
  {
    /* the final colon's spacing isn't quite the same */
    int Seconds = GetRTCSEC();
    msd = Seconds / 10;
    lsd = Seconds % 10;

    Col +=2;
    WriteTimeColon(Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);

  }
  else /* now things starting getting fun....*/
  {
    DisplayAmPm();

    if ( QueryBluetoothOn() == 0 )
    {
      CopyColumnsIntoMyBuffer(pBluetoothOffIdlePageIcon,
                              IDLE_PAGE_ICON_STARTING_ROW,
                              IDLE_PAGE_ICON_SIZE_IN_ROWS,
                              IDLE_PAGE_ICON_STARTING_COL,
                              IDLE_PAGE_ICON_SIZE_IN_COLS);
    }
    else if ( QueryPhoneConnected() == 0 )
    {
      CopyColumnsIntoMyBuffer(pPhoneDisconnectedIdlePageIcon,
                              IDLE_PAGE_ICON_STARTING_ROW,
                              IDLE_PAGE_ICON_SIZE_IN_ROWS,
                              IDLE_PAGE_ICON_STARTING_COL,
                              IDLE_PAGE_ICON_SIZE_IN_COLS);
    }
    else
    {
      if ( QueryBatteryCharging() )
      {
        CopyColumnsIntoMyBuffer(pBatteryChargingIdlePageIconType2,
                                IDLE_PAGE_ICON2_STARTING_ROW,
                                IDLE_PAGE_ICON2_SIZE_IN_ROWS,
                                IDLE_PAGE_ICON2_STARTING_COL,
                                IDLE_PAGE_ICON2_SIZE_IN_COLS);
      }
      else
      {
        unsigned int bV = 3500;

        if ( bV < 3500 )
        {
          CopyColumnsIntoMyBuffer(pLowBatteryIdlePageIconType2,
                                  IDLE_PAGE_ICON2_STARTING_ROW,
                                  IDLE_PAGE_ICON2_SIZE_IN_ROWS,
                                  IDLE_PAGE_ICON2_STARTING_COL,
                                  IDLE_PAGE_ICON2_SIZE_IN_COLS);
        }
        else
        {
          DisplayDayOfWeek();
          DisplayDate();
        }
      }
    }
  }
}

static void DisplayAmPm(void)
{
  /* don't display am/pm in 24 hour mode */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    int Hour = GetRTCHOUR();

    unsigned char const *pFoo;

    if ( Hour >= 12 )
    {
      pFoo = Pm;
    }
    else
    {
      pFoo = Am;
    }

    WriteFoo(pFoo,0,8);
  }

}


static void DisplayDayMonth(int First, int Second,  unsigned char row)
{
	/* shift bit so that it lines up with AM/PM and Day of Week */
	WriteSpriteDigit(First / 10, row, 8, -1);
	/* shift the bits so we can fit a / in the middle */
	WriteSpriteDigit(First % 10, row, 9, -1);
	WriteSpriteDigit(Second / 10, row, 10, 1);
	WriteSpriteDigit(Second % 10, row, 11, 0);
	DisplayDataSeparator(row, 9);
}


static void DisplayYear(int year, unsigned char row, unsigned char col)
{
    unsigned int bar = 1000;
    unsigned int temp = 0;
    unsigned char i;
    for (i = 0; i < 4; i++)
    {
        temp = year / bar;
        WriteSpriteDigit(temp,row,col+i,0);
        year = year % bar;
        bar = bar / 10;
    }
}

static void DisplayDayOfWeek(void)
{
  int DayOfWeek = GetRTCDOW();

  unsigned char const *pFoo = DaysOfWeek[DayOfWeek];
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    WriteFoo(pFoo,10,8);
  }
  else
  {
	// move it up so we can fit the year
    WriteFoo(pFoo,0,8);
  }
}

static void DisplayDate(void)
{
  if ( QueryFirstContact() )
  {
    int First;
    int Second;

    /* determine if month or day is displayed first */
    if ( GetDateFormat() == MONTH_FIRST )
    {
      First = GetRTCMON();
      Second = GetRTCDAY();
    }
    else
    {
      First = GetRTCDAY();
      Second = GetRTCMON();
    }
    if ( GetTimeFormat() == TWELVE_HOUR )
    {
    	DisplayDayMonth(First, Second, 20);
    }
    else
    {
    	int year = GetRTCYEAR();
    	DisplayDayMonth(First, Second, 10);

    	/* Write the year */
    	DisplayYear(year, 20, 8);
    }
  }
}




void DrawSimpleIdleScreen(void)
{
  unsigned char msd;
  unsigned char lsd;

  unsigned char Row = 6;
  unsigned char Col = 0;

  int Minutes;
  /* display hour */
  int Hour = GetRTCHOUR();
  if (Hour < 0 || Hour > 23) {
    Hour = 0;
  }

  /* if required convert to twelve hour format */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    if ( Hour == 0 )
    {
      Hour = 12;
    }
    else if ( Hour > 12 )
    {
      Hour -= 12;
    }
  }

  msd = Hour / 10;
  lsd = Hour % 10;

  /* if first digit is zero then leave location blank */
  if ( msd != 0 )
  {
    WriteTimeDigit(msd, Row, Col, LEFT_JUSTIFIED);
  }
  Col += 1;
  WriteTimeDigit(lsd, Row, Col, RIGHT_JUSTIFIED);
  Col += 2;

  /* the colon takes the first 5 bits on the byte*/
  WriteTimeColon(Row,Col,RIGHT_JUSTIFIED);
  Col += 1;

  /* display minutes */
  Minutes = GetRTCMIN();
  if (Minutes < 0 || Minutes > 59) {
    Minutes = 0;
  }
  msd = Minutes / 10;
  lsd = Minutes % 10;

  WriteTimeDigit(msd, Row, Col, RIGHT_JUSTIFIED);
  Col += 2;
  WriteTimeDigit(lsd, Row, Col, LEFT_JUSTIFIED);

  if ( nvDisplaySeconds )
  {
    /* the final colon's spacing isn't quite the same */
    int Seconds = GetRTCSEC();

    msd = Seconds / 10;
    lsd = Seconds % 10;

    Col +=2;
    WriteTimeColon(Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);

  }
  else
  {
    DisplayAmPm();
    DisplayDayOfWeek();
    DisplayDate();

  }

}

static void DrawConnectionScreen()
{
	unsigned char const* pSwash;

    unsigned char row;
    unsigned char col;

	/* this is part of the idle update
	 * timing is controlled by the idle update timer
	 * buffer was already cleared when drawing the time
	 */
	etConnectionState cs = QueryConnectionState();
	switch (cs)
	{
	case RadioOn:
		if (QueryValidPairingInfo())
		{
			pSwash = pBootPageConnectionSwash;
		}
		else
		{
			pSwash = pBootPagePairingSwash;
		}
		break;
	case Paired:
		pSwash = pBootPageConnectionSwash;
		break;
	case Connected:
		//pSwash = pBootPageConnectionSwash;
		//break;
		// Think we should do something here?
		pSwash = pBootPageUnknownSwash;
		break;
	case Initializing:
	case ServerFailure:
	case RadioOff:
	case RadioOffLowBattery:
	case ShippingMode:
	default:
		pSwash = pBootPageBluetoothOffSwash;
		break;
	}

  CopyRowsIntoMyBuffer(pSwash ,WATCH_DRAWN_IDLE_BUFFER_ROWS+1, 32);

  /* characters are 10h then add space of 2 lines */
  row = 65;
  col = 0;
  col = WriteString(GetLocalBluetoothAddressString(),row,col,DONT_ADD_SPACE_AT_END);

  /* add the firmware version */
  row = 75;
  col = 0;
  col = WriteString("App", row, col, ADD_SPACE_AT_END);
  col = WriteString("0.0.1", row, col, ADD_SPACE_AT_END);

  /* and the stack version */
  row = 85;
  col = 0;
  col = WriteString("Stack", row, col, ADD_SPACE_AT_END);
  col = WriteString("n/a", row, col, ADD_SPACE_AT_END);

}

static int IdleUpdateHandler(struct IdleInfo *Info)
{
	/* select between 1 second and 1 minute */
	int IdleUpdateTime;

	StopAllDisplayTimers();

	if (GetDisplaySeconds())
	{
		IdleUpdateTime = ONE_SECOND;
	}
	else
	{
		IdleUpdateTime = ONE_SECOND * 60;
	}

	/* setup a timer to determine when to draw the screen again */
	SetupOneSecondTimer(Info->IdleModeTimerId, IdleUpdateTime, REPEAT_FOREVER,
			IdleUpdate, NO_MSG_OPTIONS);

	StartOneSecondTimer(Info->IdleModeTimerId);

	/* determine if the bottom of the screen should be drawn by the watch */
	if (QueryFirstContact())
	{
        tHostMsg* pOutgoingMsg;
		/*
		 * immediately update the screen
		 */
		if (GetIdleBufferConfiguration() == WATCH_CONTROLS_TOP)
		{
			/* only draw watch part */
			FillMyBuffer(STARTING_ROW, WATCH_DRAWN_IDLE_BUFFER_ROWS, 0x00);
			DrawIdleScreen();
//			PrepareMyBufferForLcd(STARTING_ROW, WATCH_DRAWN_IDLE_BUFFER_ROWS);
			SendMyBufferToLcd(WATCH_DRAWN_IDLE_BUFFER_ROWS);
		}

		/* now update the remainder of the display */
		/*! make a dirty flag for the idle page drawn by the phone
		 * set it whenever watch uses whole screen
		 */
		BPL_AllocMessageBuffer(&pOutgoingMsg);
		pOutgoingMsg->Type = UpdateDisplay;
		pOutgoingMsg->Options = IDLE_MODE | DONT_ACTIVATE_DRAW_BUFFER;
		RouteMsg(&pOutgoingMsg);

		return IDLE_UPDATE_TOP_ONLY;
	}
	FillMyBuffer(STARTING_ROW, NUM_LCD_ROWS, 0x00);
	DrawSimpleIdleScreen();
	DrawConnectionScreen();

	return IDLE_UPDATE_FULL_SCREEN;
}


static void DontChangeButtonConfiguration(void)
{

}


void IdlePageMainConfigButtons(struct IdleInfo *Info)
{

	  //etConnectionState cs = QueryConnectionState();

	  //switch (cs)
	  //{
	  //case Initializing:       CurrentIdlePage = BluetoothOffPage;              break;
	  //case ServerFailure:      CurrentIdlePage = BluetoothOffPage;              break;
	  //case RadioOn:            CurrentIdlePage = RadioOnWithoutPairingInfoPage; break;
	  //case Paired:             CurrentIdlePage = RadioOnWithPairingInfoPage;    break;
	  //case Connected:          CurrentIdlePage = NormalPage;                    break;
	  //case RadioOff:           CurrentIdlePage = BluetoothOffPage;              break;
	  //case RadioOffLowBattery: CurrentIdlePage = BluetoothOffPage;              break;
	  //case ShippingMode:       CurrentIdlePage = BluetoothOffPage;              break;
	  //default:                 CurrentIdlePage = BluetoothOffPage;              break;
	etConnectionState cs = QueryConnectionState();
	switch (cs)
	{
	case RadioOff:
	case RadioOffLowBattery:
	case ShippingMode:
	case Initializing:
	case ServerFailure:
	case RadioOn:

        break;


	case Paired:
	case Connected:
    EnableButtonAction(IDLE_MODE,
                       SW_F_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       WatchStatusMsg,
                       RESET_DISPLAY_TIMER);

    EnableButtonAction(IDLE_MODE,
                       SW_E_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       ListPairedDevicesMsg,
                       NO_MSG_OPTIONS);

    EnableButtonAction(IDLE_MODE,
                       SW_C_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       MenuModeMsg,
                       MENU_MODE_OPTION_PAGE1);

    EnableButtonAction(IDLE_MODE,
                       SW_B_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       ToggleSecondsMsg,
                       TOGGLE_SECONDS_OPTIONS_UPDATE_IDLE);

    EnableButtonAction(IDLE_MODE,
                       SW_A_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       RESET_DISPLAY_TIMER);
	}
}



const unsigned char Am[10*4] =
{
0x00,0x00,0x9C,0xA2,0xA2,0xA2,0xBE,0xA2,0xA2,0x00,
0x00,0x00,0x08,0x0D,0x0A,0x08,0x08,0x08,0x08,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char Pm[10*4] =
{
0x00,0x00,0x9E,0xA2,0xA2,0x9E,0x82,0x82,0x82,0x00,
0x00,0x00,0x08,0x0D,0x0A,0x08,0x08,0x08,0x08,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char DaysOfWeek[7][10*4] =
{
0x00,0x00,0x9C,0xA2,0x82,0x9C,0xA0,0xA2,0x1C,0x00,
0x00,0x00,0x28,0x68,0xA8,0x28,0x28,0x28,0x27,0x00,
0x00,0x00,0x02,0x02,0x02,0x03,0x02,0x02,0x02,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x22,0xB6,0xAA,0xA2,0xA2,0xA2,0x22,0x00,
0x00,0x00,0x27,0x68,0xA8,0x28,0x28,0x28,0x27,0x00,
0x00,0x00,0x02,0x02,0x02,0x03,0x02,0x02,0x02,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x88,0x88,0x88,0x88,0x88,0x08,0x00,
0x00,0x00,0xE8,0x28,0x28,0xE8,0x28,0x28,0xE7,0x00,
0x00,0x00,0x03,0x00,0x00,0x01,0x00,0x00,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xA2,0xA2,0xAA,0xAA,0xAA,0xAA,0x94,0x00,
0x00,0x00,0xEF,0x20,0x20,0x27,0x20,0x20,0xEF,0x00,
0x00,0x00,0x01,0x02,0x02,0x02,0x02,0x02,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x88,0x88,0x88,0x88,0x88,0x88,0x00,
0x00,0x00,0x28,0x28,0x28,0x2F,0x28,0x28,0xC8,0x00,
0x00,0x00,0x7A,0x8A,0x8A,0x7A,0x4A,0x8A,0x89,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x82,0x82,0x9E,0x82,0x82,0x82,0x00,
0x00,0x00,0xC7,0x88,0x88,0x87,0x84,0x88,0xC8,0x00,
0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x1C,0xA2,0x82,0x9C,0xA0,0xA2,0x9C,0x00,
0x00,0x00,0xE7,0x88,0x88,0x88,0x8F,0x88,0x88,0x00,
0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


