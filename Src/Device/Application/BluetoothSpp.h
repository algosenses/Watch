#ifndef BLUETOOTH_SPP_H
#define BLUETOOTH_SPP_H

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle BluetoothRxQueue;
extern xQueueHandle BluetoothTxQueue;

void InitializeBluetoothSpp(void);

#endif  // BLUETOOTH_SPP_H
