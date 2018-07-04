#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Messages.h"
#include "DebugUart.h"
#include "LcdTask.h"
#include "LcdDisplay.h"
#include "BufferPool.h"
#include "CommandTask.h"
#include "Background.h"
#include "BluetoothSpp.h"
#include "hal_rtc.h"

/* The following is responsible for initializing the target hardware.*/
static void ConfigureHardware(void)
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }

	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	IntMasterEnable();

    InitDebugUart();
}

/* The following function exists to put the MCU to sleep when in the idle task. */
void vApplicationIdleHook(void)
{
}

int main(void)
{
    /* Order of initialization is important */

    ConfigureHardware();

    PrintString("\r\n+===================+\r\n");
    PrintString(    "|  Bluetooth Watch  |\r\n");
    PrintString(    "+===================+\r\n");

    InitializeBufferPool();

    InitializeCommandTask();

    InitializeBluetoothSpp();

    InitializeRealTimeClock();

    InitializeBackgroundTask();

    InitializeDisplayTask();

    InitializeLcdTask();

    /* Start the Task Scheduler. */
    vTaskStartScheduler();

    /* if vTaskStartScheduler exits an error occured. */
    PrintString("Program Error\r\n");
}

void vApplicationTickHook(void)
{
}
