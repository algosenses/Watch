#ifndef METAWATCH_IDLE_PAGE_MAIN_H
#define METAWATCH_IDLE_PAGE_MAIN_H

/* For External Config Menu */
void InitializeDisplaySeconds(void);
int GetDisplaySeconds(void);
void ToggleSecondsHandler(void);

extern const struct IdlePage IdlePageMain;

void InitIdlePageMain(void);

#endif /* METAWATCH_IDLE_PAGE_MAIN_H */
