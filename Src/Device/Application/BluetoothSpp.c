#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_uart.h"
#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"
#include "uart.h"

#include <string.h>
#include "Messages.h"
#include "SerialProfile.h"
#include "MessageQueues.h"
#include "BufferPool.h"
#include "DebugUart.h"
#include "BluetoothSpp.h"

#define SPP_TASK_QUEUE_LENGTH       8
#define BT_TASK_QUEUE_LENGTH        (64)
#define BT_TASK_STACK_DEPTH	        (configMINIMAL_STACK_SIZE + 90)
#define BT_TASK_PRIORITY            (tskIDLE_PRIORITY + 1)

xTaskHandle BTRxTaskHandle;
xTaskHandle BTTxTaskHandle;
xQueueHandle BluetoothRxQueue;

static tHostMsg* pBluetoothSppMsg;

/* a buffer is used so that the code does not have to wait for the uart
 * to be empty
 */
#define BT_TX_BUFFER_SIZE  (128)
static unsigned char TxBuffer[BT_TX_BUFFER_SIZE];
static volatile unsigned char WriteIndex;
static volatile unsigned char ReadIndex;
static volatile unsigned char TxCount;

static xSemaphoreHandle BtTxMutex;

static void BluetoothSppMessageHandler(tHostMsg* pMsg);

enum {
    MSG_RECV_RUNNING,
    MSG_RECV_STOP,
    MSG_RECV_DATA_LINK_ESCAPE,
};

#define MSG_START_BYTE_VALUE        HOST_MSG_START_FLAG
#define DATA_LINK_ESCAPE_VALUE      (0x10)

static tHostMsg BtRecvMsg;
static volatile unsigned char BtMsgLastRecvState = MSG_RECV_STOP;
static volatile unsigned char DataLinkEscape = 0;

#define TxFIFOFull()            (HWREG(UART1_BASE + UART_O_FR) & UART_FR_TXFF)
#define UART1_DATA_REG           HWREG(UART1_BASE + UART_O_DR)
#define BtSppPutChar(c)          UARTCharPut(UART1_BASE, c)

static void IncrementWriteIndex(void)
{
    WriteIndex++;
    if (WriteIndex == BT_TX_BUFFER_SIZE) {
        WriteIndex = 0;
    }
}

static void IncrementReadIndex(void)
{
    ReadIndex++;
    if (ReadIndex == BT_TX_BUFFER_SIZE) {
        ReadIndex = 0;
    }
}

void BtRxTxIntHandler(void)
{
    unsigned long ulStatus;
    unsigned char ucData;

    ulStatus = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, ulStatus);

    if (ulStatus & UART_INT_TX) {
        while (TxCount > 0 && !TxFIFOFull()) {
            UART1_DATA_REG = TxBuffer[ReadIndex];
            IncrementReadIndex();
            TxCount--;
        }
    }

    if (ulStatus & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(UART1_BASE)) {
            ucData = UARTCharGet(UART1_BASE);
            xQueueSendFromISR(BluetoothRxQueue, &ucData, 0);
        }
    }
}

static void WriteTxBuffer_(signed char * const pBuf, unsigned char Size)
{
    unsigned char i = 0;
    unsigned char LocalCount = TxCount;

    /* if there isn't enough room in the buffer then characters are lost */
    while (i < Size && LocalCount < BT_TX_BUFFER_SIZE) {
        TxBuffer[WriteIndex] = pBuf[i++];
        IncrementWriteIndex();
        LocalCount++;
    }

    /* update the count (which can be decremented in th ISR) and
     * start sending characters if the UART is currently idle */
    if (i > 0) {
        UARTIntDisable(UART1_BASE, UART_INT_TX);   /* disable uart tx interrupt */
        TxCount += i;
        if (!TxFIFOFull()) {
            UART1_DATA_REG = TxBuffer[ReadIndex];
            IncrementReadIndex();
            TxCount--;
        }
        UARTIntEnable(UART1_BASE, UART_INT_TX);
    }
}

static void WriteTxBuffer(unsigned char * const pBuf, unsigned char Size)
{
    unsigned char i;

    for (i = 0; i < Size; i++) {
        UARTCharPut(UART1_BASE, pBuf[i++]);
    }
}


static void BTRxTask(void *pvParameters)
{
    unsigned char ucData;
    unsigned char *pMsgRecvBuffer = &(BtRecvMsg.startByte);
    unsigned char MsgRecvIndex = 0;
    tHostMsg* pOutgoingMsg;

    while (1) {
        if (pdTRUE == xQueueReceive(BluetoothRxQueue, &ucData, portMAX_DELAY)) {
            switch (BtMsgLastRecvState) {
            case MSG_RECV_STOP:
                if (ucData == MSG_START_BYTE_VALUE) {
                    MsgRecvIndex = 0;
                    BtMsgLastRecvState = MSG_RECV_RUNNING;
                    memset(pMsgRecvBuffer, 0, HOST_MSG_BUFFER_LENGTH);
                    pMsgRecvBuffer[MsgRecvIndex++] = ucData;
                }
                break;

            case MSG_RECV_RUNNING:
                if (ucData != DATA_LINK_ESCAPE_VALUE) {

                    pMsgRecvBuffer[MsgRecvIndex++] = ucData;

                    if (MsgRecvIndex > HOST_MSG_BUFFER_LENGTH) {
                        BtMsgLastRecvState = MSG_RECV_STOP;
                        MsgRecvIndex = 0;
                    }

                    if (BtRecvMsg.Length <= HOST_MSG_BUFFER_LENGTH && MsgRecvIndex == BtRecvMsg.Length) {
                        unsigned char i;
                        unsigned char *out;

                        BtMsgLastRecvState = MSG_RECV_STOP; /* receive a frame */
                        MsgRecvIndex = 0;

                        BPL_AllocMessageBuffer(&pOutgoingMsg);
                        out = (unsigned char *)pOutgoingMsg;
                        for (i = 0; i < BtRecvMsg.Length; i++) {
                            out[i] = pMsgRecvBuffer[i];
                           // PrintHex(out[i]);
                        }

                        RouteMsg(&pOutgoingMsg);
                    }
                } else {
                    BtMsgLastRecvState = MSG_RECV_DATA_LINK_ESCAPE;
                }
                break;

            case MSG_RECV_DATA_LINK_ESCAPE:
                pMsgRecvBuffer[MsgRecvIndex++] = ucData;
                BtMsgLastRecvState = MSG_RECV_RUNNING;
            }
        }
    }
}

static void BTTxTask(void *pvParameters)
{
//    WriteTxBuffer("AT");
//    WriteTxBuffer("AT+NAMEAirRemote");
//    WriteTxBuffer("AT+BAUD8");

    if ( QueueHandles[SPP_TASK_QINDEX] == 0 )
    {
        PrintString("Bluetooth SPP Queue not created!\r\n");
    }

    while (1) {
        if ( pdTRUE == xQueueReceive(QueueHandles[SPP_TASK_QINDEX],
                                &pBluetoothSppMsg, portMAX_DELAY ) )
        {
            BluetoothSppMessageHandler(pBluetoothSppMsg);

            BPL_FreeMessageBuffer(&pBluetoothSppMsg);
        }
    }
}

static void SendHostMessage(tHostMsg* pMsg)
{
    unsigned char i;
    unsigned char *pData = (unsigned char *)pMsg;
    unsigned char Esc = DATA_LINK_ESCAPE_VALUE;

    BtSppPutChar(pData[0]);

    for (i = 1; i < pMsg->Length; i++) {
        if (pData[i] == MSG_START_BYTE_VALUE || pData[i] == DATA_LINK_ESCAPE_VALUE) {
//            BtSppPutChar(Esc);    // ???
        }
        BtSppPutChar(pData[i]);
    }
}

/*! Handle the messages routed to the bluetooth spp task */
static void BluetoothSppMessageHandler(tHostMsg* pMsg)
{
    eMessageType Type = (eMessageType)pMsg->Type;
    switch (Type) {
    case GetDeviceTypeResponse:
        SendHostMessage(pMsg);
        break;
    }
}

static void InitBtUart(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

//    IntRegister(INT_UART1, BtRxTxIntHandler);

//    UARTFIFODisable(UART1_BASE);

//    UARTIntEnable(UART1_BASE, UART_INT_TX);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART1);
}

void InitializeBluetoothSpp(void)
{
    InitBtUart();

    BluetoothRxQueue = xQueueCreate(BT_TASK_QUEUE_LENGTH, sizeof(char) );

    QueueHandles[SPP_TASK_QINDEX] =
            xQueueCreate( SPP_TASK_QUEUE_LENGTH, MESSAGE_QUEUE_ITEM_SIZE );


    // task function, task name, stack len , task params, priority, task handle
    xTaskCreate(BTRxTask,
              "BTRxTask",
              BT_TASK_STACK_DEPTH,
              NULL,
              BT_TASK_PRIORITY,
              &BTRxTaskHandle);

    xTaskCreate(BTTxTask,
                "BTTxTask",
                BT_TASK_STACK_DEPTH,
                NULL,
                BT_TASK_PRIORITY,
                &BTTxTaskHandle);
}

unsigned char QueryPhoneConnected(void)
{
    return 1;
}

unsigned char QueryBluetoothOn(void)
{
    return 1;
}

etConnectionState QueryConnectionState(void)
{
    return Connected;
}

unsigned char QueryValidPairingInfo(void)
{
    return 0;
}
