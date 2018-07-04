#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_uart.h"
#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"
#include "uart.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include "DebugUart.h"
#include "BluetoothSpp.h"

/* a buffer is used so that the code does not have to wait for the uart
 * to be empty
 */
#define TX_BUFFER_SIZE  (128)
static unsigned char TxBuffer[TX_BUFFER_SIZE];
static volatile unsigned char WriteIndex;
static volatile unsigned char ReadIndex;
static volatile unsigned char TxCount;

signed char ConversionString[6];

static xSemaphoreHandle UartMutex;

#define TxFIFOFull()            (HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFF)
#define UART0_DATA_REG           HWREG(UART0_BASE + UART_O_DR)

static void IncrementWriteIndex(void)
{
    WriteIndex++;
    if (WriteIndex == TX_BUFFER_SIZE) {
        WriteIndex = 0;
    }
}

static void IncrementReadIndex(void)
{
    ReadIndex++;
    if (ReadIndex == TX_BUFFER_SIZE) {
        ReadIndex = 0;
    }
}

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void DebugUartIntHandler(void)
{
    unsigned long ulStatus;
    unsigned char ucData;
    //
    // Get the interrrupt status.
    //
    ulStatus = UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART0_BASE, ulStatus);

    if (ulStatus & UART_INT_TX) {
        while (TxCount > 0 && !TxFIFOFull()) {
            UART0_DATA_REG = TxBuffer[ReadIndex];
            IncrementReadIndex();
            TxCount--;
        }
    }

    if (ulStatus & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(UART0_BASE)) {
            ucData = UARTCharGet(UART0_BASE);
//            xQueueSendFromISR(BluetoothTxQueue, &ucData, 0);
        }
    }
}

void InitDebugUart(void)
{
    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    // bug???
//    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);
//    UARTFIFOEnable(UART0_BASE);
//    UARTTxIntModeSet(UART0_BASE, UART_TXINT_MODE_EOT);

    UARTFIFODisable(UART0_BASE);

    //
    // Enable the UART interrupt.
    //
    UARTIntEnable(UART0_BASE, UART_INT_TX);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART0);
    UARTEnable(INT_UART0);

    WriteIndex = 0;
    ReadIndex = 0;
    TxCount = 0;

    UartMutex = xSemaphoreCreateMutex();

    xSemaphoreGive(UartMutex);
}

static void WriteTxBuffer(signed char * const pBuf)
{
    unsigned char i = 0;
    unsigned char LocalCount = TxCount;

    /* if there isn't enough room in the buffer then characters are lost */
    while (pBuf[i] != '\0' && LocalCount < TX_BUFFER_SIZE) {
        TxBuffer[WriteIndex] = pBuf[i++];
        IncrementWriteIndex();
        LocalCount++;
    }

    /* update the count (which can be decremented in th ISR) and
     * start sending characters if the UART is currently idle */
    if (i > 0) {
        UARTIntDisable(UART0_BASE, UART_INT_TX);   /* disable uart tx interrupt */
        TxCount += i;
        if (!TxFIFOFull()) {
            UART0_DATA_REG = TxBuffer[ReadIndex];
            IncrementReadIndex();
            TxCount--;
        }
        UARTIntEnable(UART0_BASE, UART_INT_TX);
    }
}

/* block version of debug function */
static void WriteTxBuffer_(signed char * const pBuf)
{
    unsigned int i = 0;

    UARTIntDisable(UART0_BASE, UART_INT_TX);

    while (pBuf[i] != 0) {
        UARTCharPut(UART0_BASE, pBuf[i++]);
    }
}

/*! convert a 16 bit value into a string */
void ToDecimalString(unsigned int Value, signed char * pString)
{
  unsigned int bar = 10000;
  unsigned char index = 0;
  unsigned int temp = 0;
  unsigned char first = 0;
  unsigned char i = 0;

  for (i = 0; i < 5; i++ )
  {
    temp = Value / bar;

    if ( temp > 0 || first || i == 4)
    {
      pString[index++] = temp + '0';
      first = 1;
    }

    Value = Value % bar;

    bar = bar / 10;

  }

  pString[index] = 0;

}


/*! convert a 16 bit value into a hexadecimal string */
void ToHexString(unsigned int Value, signed char * pString)
{
  unsigned char parts[4];
  unsigned char first = 0;
  unsigned char index = 0;
  signed char i;

  parts[3] = (0xF000 & Value) >> 12;
  parts[2] = (0x0F00 & Value) >> 8;
  parts[1] = (0x00F0 & Value) >> 4;
  parts[0] = (0x000F & Value);


  for ( i = 3; i > -1; i-- )
  {
    if ( parts[i] > 0 || first || i == 0 )
    {
      if ( parts[i] > 9 )
      {
        pString[index++] = parts[i] + 'A' - 10;
      }
      else
      {
        pString[index++] = parts[i] + '0';
      }
      first = 1;
    }

  }

  pString[index] = 0;

}

/* These functions were created because the routines that came with the stack
 * broke down when there was memory issues (and there is nothing more annoying
 * than getting invalid debug information)
 */
void PrintString(signed char * const pString)
{
    xSemaphoreTake(UartMutex, portMAX_DELAY);
    WriteTxBuffer(pString);
    xSemaphoreGive(UartMutex);
}

void PrintString2(signed char * const pString1,signed char * const pString2)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString1);
    WriteTxBuffer(pString2);
    xSemaphoreGive(UartMutex);
}

void PrintString3(signed char * const pString1,
                  signed char * const pString2,
                  signed char * const pString3)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString1);
    WriteTxBuffer(pString2);
    WriteTxBuffer(pString3);
    xSemaphoreGive(UartMutex);
}

void PrintDecimal(unsigned int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    ToDecimalString(Value,ConversionString);
    WriteTxBuffer(ConversionString);
    xSemaphoreGive(UartMutex);
}

void PrintHex(unsigned int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer("0x");
    ToHexString(Value, ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer(" ");
    xSemaphoreGive(UartMutex);
}

void PrintDecimalAndNewline(unsigned int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    ToDecimalString(Value,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintSignedDecimalAndNewline(signed int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    if ( Value < 0 )
    {
        Value = ~Value + 1;
    }
    ToDecimalString(Value,ConversionString);
    WriteTxBuffer("-");
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintStringAndDecimal(signed char * const pString,unsigned int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
//    IntMasterDisable();
    WriteTxBuffer(pString);
    ToDecimalString(Value,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
//    IntMasterEnable();
    xSemaphoreGive(UartMutex);
}

void PrintStringAndSpaceAndDecimal(signed char * const pString,unsigned int Value)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString);
    WriteTxBuffer(" ");
    ToDecimalString(Value,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintStringAndTwoDecimals(signed char * const pString1,
                               unsigned int Value1,
                               signed char * const pString2,
                               unsigned int Value2)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString1);
    ToDecimalString(Value1,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer(" ");

    WriteTxBuffer(pString2);
    ToDecimalString(Value2,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintStringSpaceAndTwoDecimals(signed char * const pString1,
                                    unsigned int Value1,
                                    unsigned int Value2)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString1);
    WriteTxBuffer(" ");
    ToDecimalString(Value1,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer(" ");
    ToDecimalString(Value2,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintStringSpaceAndThreeDecimals(signed char * const pString1,
                                      unsigned int Value1,
                                      unsigned int Value2,
                                      unsigned int Value3)
{
    xSemaphoreTake(UartMutex,portMAX_DELAY);
    WriteTxBuffer(pString1);
    WriteTxBuffer(" ");
    ToDecimalString(Value1,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer(" ");
    ToDecimalString(Value2,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer(" ");
    ToDecimalString(Value3,ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
    xSemaphoreGive(UartMutex);
}

void PrintStringAndHex(signed char * const pString, unsigned int Value)
{
    xSemaphoreTake(UartMutex, portMAX_DELAY);
//    IntMasterDisable();
    WriteTxBuffer(pString);
    ToHexString(Value, ConversionString);
    WriteTxBuffer(ConversionString);
    WriteTxBuffer("\r\n");
//    IntMasterEnable();
    xSemaphoreGive(UartMutex);
}

