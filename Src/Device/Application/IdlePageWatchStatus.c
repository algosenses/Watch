#include <stddef.h>
#include "Messages.h" //IDLE_MODE + msgs
#include "Buttons.h"  // BUTTON_STATE
#include "hal_board_type.h"// SW_
#include "Display.h"
#include "LcdDisplay.h"
#include "LcdBuffer.h"
#include "Icons.h"
#include "OneSecondTimers.h"
#include "BlueTooth.h"
#include "Adc.h"
#include "hal_battery.h"
#include "IdlePage.h"
#include "IdlePageWatchStatus.h"

void IdlePageWatchStatusConfigButtons(struct IdleInfo *Info);
int IdlePageWatchStatusHandler(struct IdleInfo *Info);

const struct IdlePage IdlePageWatchStatus = {
	.Start = NULL,
	.Stop = NULL,
	.Handler = IdlePageWatchStatusHandler,
	.ConfigButtons = IdlePageWatchStatusConfigButtons, };

void IdlePageWatchStatusConfigButtons(struct IdleInfo *Info)
{
    /* map this mode's entry button to go back to the idle mode */
    EnableButtonAction(IDLE_MODE,
                       SW_F_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       IdleUpdate,
                       RESET_DISPLAY_TIMER);

    EnableButtonAction(IDLE_MODE,
                       SW_E_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       ListPairedDevicesMsg,
                       NO_MSG_OPTIONS);

    /* led is already assigned */

    EnableButtonAction(IDLE_MODE,
                       SW_C_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       MenuModeMsg,
                       MENU_MODE_OPTION_PAGE1);

    DisableButtonAction(IDLE_MODE,
                        SW_B_INDEX,
                        BUTTON_STATE_IMMEDIATE);

    EnableButtonAction(IDLE_MODE,
                       SW_A_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       RESET_DISPLAY_TIMER);
}

/*
 * 	static void WatchStatusScreenHandler(void)
 */
int IdlePageWatchStatusHandler(struct IdleInfo *Info)
{
	  StopAllDisplayTimers();

	  FillMyBuffer(STARTING_ROW,NUM_LCD_ROWS,0x00);

	  /*
	   * Add Status Icons
	   */
	  unsigned char const * pIcon;

	  if ( QueryBluetoothOn() )
	  {
	    pIcon = pBluetoothOnStatusScreenIcon;
	  }
	  else
	  {
	    pIcon = pBluetoothOffStatusScreenIcon;
	  }

	  CopyColumnsIntoMyBuffer(pIcon,
	                         0,
	                         STATUS_ICON_SIZE_IN_ROWS,
	                         LEFT_STATUS_ICON_COLUMN,
	                         STATUS_ICON_SIZE_IN_COLUMNS);


	  if ( QueryPhoneConnected() )
	  {
	    pIcon = pPhoneConnectedStatusScreenIcon;
	  }
	  else
	  {
	    pIcon = pPhoneDisconnectedStatusScreenIcon;
	  }

	  CopyColumnsIntoMyBuffer(pIcon,
	                         0,
	                         STATUS_ICON_SIZE_IN_ROWS,
	                         CENTER_STATUS_ICON_COLUMN,
	                         STATUS_ICON_SIZE_IN_COLUMNS);

	  unsigned int bV = ReadBatterySenseAverage();

	  if ( QueryBatteryCharging() )
	  {
	    pIcon = pBatteryChargingStatusScreenIcon;
	  }
	  else
	  {
	    if ( bV > 4000 )
	    {
	      pIcon = pBatteryFullStatusScreenIcon;
	    }
	    else if ( bV < 3500 )
	    {
	      pIcon = pBatteryLowStatusScreenIcon;
	    }
	    else
	    {
	      pIcon = pBatteryMediumStatusScreenIcon;
	    }
	  }


	  CopyColumnsIntoMyBuffer(pIcon,
	                         0,
	                         STATUS_ICON_SIZE_IN_ROWS,
	                         RIGHT_STATUS_ICON_COLUMN,
	                         STATUS_ICON_SIZE_IN_COLUMNS);

	  unsigned char row = 27;
	  unsigned char col = 8;
	  unsigned char msd = 0;

	  msd = bV / 1000;
	  bV = bV % 1000;
	  WriteSpriteDigit(msd,row,col++,0);

	  msd = bV / 100;
	  bV = bV % 100;
	  WriteSpriteDigit(msd,row,col++,0);
	  AddDecimalPoint8w10h(row,col-2);

	  msd = bV / 10;
	  bV = bV % 10;
	  WriteSpriteDigit(msd,row,col++,0);

	  WriteSpriteDigit(bV,row,col++,0);

	  /*
	   * Add Wavy line
	   */
	  row += 12;
	  CopyRowsIntoMyBuffer(pWavyLine,row,NUMBER_OF_ROWS_IN_WAVY_LINE);


	  /*
	   * Add details
	   */

	  /* add MAC address */
	  row += NUMBER_OF_ROWS_IN_WAVY_LINE+2;
	  col = 0;
	  WriteString(GetLocalBluetoothAddressString(),row,col,DONT_ADD_SPACE_AT_END);

	  /* add the firmware version */
	  row += 12;
	  col = 0;
	  col = WriteString("App",row,col,ADD_SPACE_AT_END);
	  col = WriteString(VERSION_STRING,row,col,ADD_SPACE_AT_END);

	  /* stack version */
	  row += 12;
	  col = 0;
	  col = WriteString("Stack",row,col,ADD_SPACE_AT_END);
	  col = WriteString(GetStackVersion(),row,col,ADD_SPACE_AT_END);

	  /* add msp430 revision */
	  row +=12;
	  col = 0;
	  col = WriteString("MSP430 Rev",row,col,DONT_ADD_SPACE_AT_END);
	  WriteSpriteChar(GetHardwareRevision(),row,col++);

	  /* display entire buffer */
          /* Move out also This seems to be common to all idle pages */
	  /* PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
	  SendMyBufferToLcd(NUM_LCD_ROWS);*/

          /* Move this to common code outside the handler 
           * if we get strage stuff happening then move it back before the timer
           * Need to think about this 
          
	  CurrentIdlePage = WatchStatusPage;
	  ConfigureIdleUserInterfaceButtons();
          */
	  /* refresh the status page once a minute */
	  SetupOneSecondTimer(Info->IdleModeTimerId,
	                      ONE_SECOND*60,
	                      NO_REPEAT,
	                      WatchStatusMsg,
	                      NO_MSG_OPTIONS);

	  StartOneSecondTimer(Info->IdleModeTimerId);
    return IDLE_UPDATE_FULL_SCREEN;
}

