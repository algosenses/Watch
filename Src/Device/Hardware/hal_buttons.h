#ifndef HAL_BUTTONS_H
#define HAL_BUTTONS_H

/* the switch does not count */
#define NUMBER_OF_BUTTONS ( 7 )

void CONFIGURE_BUTTON_PINS(void);
unsigned char BUTTON_PORT_IN(void);
unsigned char ReadButtonInterruptFlags(void);

#endif /* HAL_BUTTONS_H */
