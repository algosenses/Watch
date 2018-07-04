#include <stddef.h>
#include "LcdBuffer.h"
#include "LcdDisplay.h"
#include "IdlePage.h"
#include "Messages.h" //IDLE_MODE + msgs
#include "Buttons.h"  // BUTTON_STATE
#include "hal_board_type.h"// SW_
#include "SerialProfile.h"

static int ListPairedDevicesHandler(struct IdleInfo *Info);
static void ListPairedDevicesConfigButtons(struct IdleInfo *Info);
const struct IdlePage IdlePageListPairedDevices = {
		.Start = NULL,
		.Stop = NULL,
		.Handler = ListPairedDevicesHandler,
		.ConfigButtons = ListPairedDevicesConfigButtons, };


static unsigned char pBluetoothAddress[12+1];
static unsigned char pBluetoothName[12+1];

static int ListPairedDevicesHandler(struct IdleInfo *Info)
{
  StopAllDisplayTimers();

  unsigned char row = 0;
  unsigned char col = 0;

  /* draw entire region */
  FillMyBuffer(STARTING_ROW,NUM_LCD_ROWS,0x00);

  for(unsigned char i = 0; i < 3; i++)
  {
    unsigned char j;
    pBluetoothName[0] = 'D';
    pBluetoothName[1] = 'e';
    pBluetoothName[2] = 'v';
    pBluetoothName[3] = 'i';
    pBluetoothName[4] = 'c';
    pBluetoothName[5] = 'e';
    pBluetoothName[6] = ' ';
    pBluetoothName[7] = 'N';
    pBluetoothName[8] = 'a';
    pBluetoothName[9] = 'm';
    pBluetoothName[10] = 'e';
    pBluetoothName[11] = '1' + i;

    for(j = 0; j < sizeof(pBluetoothAddress); j++)
    {
      pBluetoothAddress[j] = '0';
    }

    QueryLinkKeys(i,pBluetoothAddress,pBluetoothName,12);

    WriteString(pBluetoothName,row,col,DONT_ADD_SPACE_AT_END);
    row += 12;

    WriteString(pBluetoothAddress,row,col,DONT_ADD_SPACE_AT_END);
    row += 12+5;

  }
  return IDLE_UPDATE_FULL_SCREEN;
}

static void ListPairedDevicesConfigButtons(struct IdleInfo *Info)
{

    EnableButtonAction(IDLE_MODE,
                       SW_F_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       WatchStatusMsg,
                       RESET_DISPLAY_TIMER);

    /* map this mode's entry button to go back to the idle mode */
    EnableButtonAction(IDLE_MODE,
                       SW_E_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       IdleUpdate,
                       NO_MSG_OPTIONS);

//    DontChangeButtonConfiguration();??

    EnableButtonAction(IDLE_MODE,
                       SW_C_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       MenuModeMsg,
                       MENU_MODE_OPTION_PAGE1);

    DisableButtonAction(IDLE_MODE,
                        SW_B_INDEX,
                        BUTTON_STATE_IMMEDIATE);

    /* map this mode's entry button to go back to the idle mode */
    EnableButtonAction(IDLE_MODE,
                       SW_A_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       RESET_DISPLAY_TIMER);
}
