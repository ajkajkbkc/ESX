
#ifndef _KALYKE_MODBUS_TCP_H
#define _KALYKE_MODBUS_TCP_H
#include "FreeRTOS.h"
#include "task.h"


#define MAX_MODBUS_ITEM_NUM    256
#define MAX_MODBUS_TCP_ITEM    16

extern bool gSOEMInProcess;
extern TaskHandle_t gModbusSendHandle;
extern void start_modbus_tcp(void);
extern void stop_modbus_tcp(void);
#endif /* _KALYKE_MODBUS_TCP_H */

