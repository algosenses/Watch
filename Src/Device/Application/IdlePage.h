#ifndef METAWATCH_IDLE_PAGE_H
#define METAWATCH_IDLE_PAGE_H

#include "hal_lcd.h"

struct IdleInfo
{
	int IdleModeTimerId;
	tLcdLine *buffer;
};

/*
 * Handle can return an int.
 * This is one of the below
 */
#define IDLE_NO_UPDATE 0x01
#define IDLE_UPDATE_FULL_SCREEN 0x2
#define IDLE_UPDATE_TOP_ONLY 0x3


struct IdlePage
{
	struct IdleInfo *Info;
	void (*Start)(struct IdleInfo *Info);
	void (*Stop)(struct IdleInfo *Info);
	int (*Handler)(struct IdleInfo *Info);
	void (*ConfigButtons)(struct IdleInfo *Info);
};

void InitIdlePage(int IdleModeTimerId,
	          tLcdLine *buffer);

//void IdlePageStart(struct IdlePage const * Page);
//void IdlePageStop(struct IdlePage const * Page);
void IdlePageHandler(struct IdlePage const * Page);
//void IdlePageConfigButtons(struct IdlePage const * Page);
const struct IdlePage * IdlePageCurrent(void);
#endif /* METAWATCH_IDLE_PAGE_H*/
