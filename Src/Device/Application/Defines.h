#ifndef DEFINES_H
#define DEFINES_H

#define BIT0    (0x1 << 0)
#define BIT1    (0x1 << 1)
#define BIT2    (0x1 << 2)
#define BIT3    (0x1 << 3)
#define BIT4    (0x1 << 4)
#define BIT5    (0x1 << 5)
#define BIT6    (0x1 << 6)
#define BIT7    (0x1 << 7)

// Button defines

#define BUTTON_TIME_COUNT_ARRAY_LEN  8
#define ALL_BUTTONS_OFF              0xFF

#define SW_A  BIT0
#define SW_B  BIT1
#define SW_C  BIT2
#define SW_D  BIT3
// Bit 4 is not used
#define SW_E  BIT5
#define SW_F  BIT6
#define SW_P  BIT7

#define SW_A_INDEX        ( 0 )
#define SW_B_INDEX        ( 1 )
#define SW_C_INDEX        ( 2 )
#define SW_D_INDEX        ( 3 )
#define SW_E_INDEX        ( 4 )
#define SW_F_INDEX        ( 5 )
#define SW_P_INDEX        ( 6 )
#define SW_UNUSED_INDEX   ( 7 )
/* the switch does not count */
#define NUMBER_OF_BUTTONS ( 7 )

#endif /* DEFINES_H */
