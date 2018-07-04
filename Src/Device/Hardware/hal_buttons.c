#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "sysctl.h"
#include "interrupt.h"
#include "gpio.h"

#include "Buttons.h"
#include "hal_buttons.h"

unsigned char ReadButtonInterruptFlags(void)
{
    unsigned char flags = 0;

    flags = ((GPIOPinIntStatus(GPIO_PORTE_BASE, true) & (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)) |
             (GPIOPinIntStatus(GPIO_PORTF_BASE, true) & GPIO_PIN_1) << 3);

    return flags;
}

void GPIOFIntHandler(void)
{
    ButtonPortIsr();

    GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_1);       /* SELECT_SW */
}

void GPIOEIntHandler(void)
{
    ButtonPortIsr();

    GPIOPinIntClear(GPIO_PORTE_BASE,
                    GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void CONFIGURE_BUTTON_PINS(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTE_BASE,
                   GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   GPIO_FALLING_EDGE );
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    IntEnable(INT_GPIOE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
    GPIOPinIntEnable(GPIO_PORTF_BASE, GPIO_PIN_1);
    IntEnable(INT_GPIOF);
}

unsigned char BUTTON_PORT_IN(void)
{
    /* Read the state of the push buttons */
    unsigned char btns = (GPIOPinRead(GPIO_PORTE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                            GPIO_PIN_2 | GPIO_PIN_3)) |
                           (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) << 3));

    btns ^= 0x1F;

    return btns;
}
