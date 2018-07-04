#include <stddef.h>
#include "IdlePage.h"
#include "LcdBuffer.h"
#include "LcdDisplay.h"
#include "IdlePageMain.h"

static struct IdleInfo Info;

struct IdlePage const * CurrentPage = NULL;

void InitIdlePage(int IdleModeTimerId, tLcdLine *buffer)
{
  Info.buffer = buffer;
  Info.IdleModeTimerId = IdleModeTimerId;

  InitIdlePageMain();
//  InitIdlePageGameOfLife();
  InitIdlePageQrCode();
}

void IdlePageStart(struct IdlePage const * Page)
{
	if(Page != NULL && Page->Start != NULL)
	{
		Page->Start(&Info);
	}
}

void IdlePageStop(struct IdlePage const * Page)
{
	if(Page != NULL && Page->Stop != NULL)
	{
		Page->Stop(&Info);
	}
}

const struct IdlePage * IdlePageCurrent(void)
{
  return CurrentPage;
}

void IdlePageHandler(struct IdlePage const * Page)
{
	if (Page != NULL && Page->Handler != NULL)
	{
	    int update;
		if (Page != CurrentPage)
		{
			if (CurrentPage != NULL && CurrentPage->Stop != NULL)
			{
				CurrentPage->Stop(&Info);
			}
			if (Page->Start != NULL)
			{
				Page->Start(&Info);
			}
			if(Page->ConfigButtons != NULL)
			{
				Page->ConfigButtons(&Info);
			}
			CurrentPage = Page;
		}
		update = Page->Handler(&Info);
		switch(update)
		{
		case IDLE_UPDATE_FULL_SCREEN:
		    PrepareMyBufferForLcd(STARTING_ROW, NUM_LCD_ROWS);
		    SendMyBufferToLcd(NUM_LCD_ROWS);
		    break;
		case IDLE_UPDATE_TOP_ONLY:
			PrepareMyBufferForLcd(STARTING_ROW,WATCH_DRAWN_IDLE_BUFFER_ROWS);
			SendMyBufferToLcd(WATCH_DRAWN_IDLE_BUFFER_ROWS);
			break;
		case IDLE_NO_UPDATE:
		default:
			break;
		}
	}
}

void IdlePageConfigButtons(struct IdlePage const * Page)
{
	if(Page != NULL && Page->ConfigButtons != NULL)
	{
		Page->ConfigButtons(&Info);
	}
}
