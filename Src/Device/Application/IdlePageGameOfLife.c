#include <string.h>

#include "Messages.h" //IDLE_MODE + msgs
#include "Buttons.h"  // BUTTON_STATE
#include "Defines.h"// SW_
#include "LcdDisplay.h"
#include "OneSecondTimers.h"
#include "IdlePage.h"

#define HEIGHT 96
#define WIDTH 96

void IdlePageGameOfLifeConfigButtons(struct IdleInfo *Info);
int IdlePageGameOfLifeHandler(struct IdleInfo *Info);;

struct IdlePage IdlePageGameOfLife;
static tLcdLine tmp[96];

void InitIdlePageGameOfLife(void)
{
    IdlePageGameOfLife.Start = NULL;
    IdlePageGameOfLife.Stop = NULL;
    IdlePageGameOfLife.Handler = IdlePageGameOfLifeHandler;
    IdlePageGameOfLife.ConfigButtons = IdlePageGameOfLifeConfigButtons;
}

static unsigned char pix_get(tLcdLine *tmp, signed char x, signed char y)
{
    unsigned char val;
	if(x < 0 || x >=  HEIGHT) return 0;
	if((y < 0) || (y >= WIDTH)) return 0;
	val = (1 << (x % 8));
	return (tmp[y].Data[(x / 8)] & val) == val;
}

static void pix_set(tLcdLine *tmp, signed char x, signed char y)
{
	int index = (x / 8);
	unsigned char val = 1 << (x % 8);
	tmp[y].Data[index] = tmp[y].Data[index] | val;
}


static void live(tLcdLine *life)
{
	unsigned char us, count;
	signed char y,x;
	memset(tmp, 0, sizeof(tmp));
	for(y = 0; y < HEIGHT; ++y)
	{
		//printf("%s\n", byte_to_binary(1 << i++));
		for(x = 0; x < WIDTH; ++x)
		{
			us = pix_get(life, x, y);
			count = pix_get(life, x + -1, y + -1) +
			pix_get(life, x + -1, y + 0) +
			pix_get(life, x + -1, y + 1) +
			pix_get(life, x + 0, y + -1) +
			pix_get(life, x + 0, y + 1) +
			pix_get(life, x + 1, y + 1) +
			pix_get(life, x + 1, y + 0) +
			pix_get(life, x + 1, y + -1);
			//printf("%i,%i,%i\n",(int)x,(int)y,(int)count);
			if(us)
			{
				if( (count == 2) || !!(count== 3))
				{
					pix_set(tmp,x,y);
				}
			}
			else
			{
				if(count == 3)
				{
					pix_set(tmp,x,y);
				}
			}
		}
	}
        for(y = 0; y < HEIGHT; ++y)
	{
            memcpy(life[y].Data, tmp[y].Data, sizeof(tmp[y].Data));
        }
}


static int IdlePageGameOfLifeHandler(struct IdleInfo *Info)
{

    StopAllDisplayTimers(); // Think we'll do this when we change pages???
    SetupOneSecondTimer(Info->IdleModeTimerId,
	                     ONE_SECOND,
	                      NO_REPEAT,
	                      BarCode,
	                      NO_MSG_OPTIONS);
    live(Info->buffer);
    StartOneSecondTimer(Info->IdleModeTimerId);
    return IDLE_UPDATE_FULL_SCREEN;
}

static void IdlePageGameOfLifeConfigButtons(struct IdleInfo *Info)
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

