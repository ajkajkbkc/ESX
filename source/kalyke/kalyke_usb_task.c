/**
  ******************************************************************************
  * @file    kalyke_usb_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-08
  * @brief   USB task
  ******************************************************************************
  */
    
#include "kalyke_usb_task.h"
    
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
    
#include <stdio.h>
#include <stdlib.h>
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
    
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif
    
#include "pin_mux.h"
#include "bsp_led.h"

//#include "FreeRTOS.h"

#include "plc_variable.h"
#include "mb.h"
#include "bsp_dct.h"
//#include "bsp_uart.h"
#include "mb.h"
#include "queue.h"
#include "plc_commonfunc.h"
#include "kalyke_tool.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static usb_status_t USB_DeviceHidGenericCallback(class_handle_t handle, uint32_t event, void *param);
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "usb_task";
TaskHandle_t gUSBTaskHandler;
#if 0
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint32_t s_GenericBuffer0[USB_HID_GENERIC_OUT_BUFFER_LENGTH >> 2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint32_t s_GenericBuffer1[USB_HID_GENERIC_OUT_BUFFER_LENGTH >> 2];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_GenericBuffer0[USB_HID_GENERIC_OUT_BUFFER_LENGTH];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_GenericBuffer1[USB_HID_GENERIC_OUT_BUFFER_LENGTH];
#endif
usb_hid_generic_struct_t g_UsbDeviceHidGeneric;
extern usb_device_class_struct_t g_UsbDeviceHidGenericConfig;
/* Set class configurations */
usb_device_class_config_struct_t g_UsbDeviceHidConfig[1] = {{
    USB_DeviceHidGenericCallback, /* HID generic class callback pointer */
    (class_handle_t)NULL,         /* The HID class handle, This field is set by USB_DeviceClassInit */
    &g_UsbDeviceHidGenericConfig, /* The HID mouse configuration, including class code, subcode, and protocol, class
                                     type,
                                     transfer type, endpoint address, max packet size, etc.
                                  */
}};
/* Set class configuration list */
usb_device_class_config_list_struct_t g_UsbDeviceHidConfigList = {
    g_UsbDeviceHidConfig, /* Class configurations */
    USB_DeviceCallback,   /* Device callback pointer */
    1U,                   /* Class count */
};
static volatile bool gIfCanSend = true;


/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    //bsp_toggle_led_ERR();
    USB_DeviceEhciIsrFunction(g_UsbDeviceHidGeneric.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    //bsp_toggle_led_RUN();
    USB_DeviceEhciIsrFunction(g_UsbDeviceHidGeneric.deviceHandle);
}

void USB_DeviceClockInit(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

static void usb_send_data(uint8_t *pData, uint16_t len)
{
    //LOGD(TAG, "Enter %s(), gIfCanSend = %u, attach = %u, hidHandle = 0x%08X", __func__, gIfCanSend, g_UsbDeviceHidGeneric.attach, g_UsbDeviceHidGeneric.hidHandle);
    if (gIfCanSend == false)
    {
        //return;
    }
    if (g_UsbDeviceHidGeneric.attach)
    {
        gIfCanSend = false;
        //strcpy((char*)g_UsbDeviceHidGeneric.sendBuffer, (char *)pData);
        usb_status_t ret = USB_DeviceHidSend(g_UsbDeviceHidGeneric.hidHandle,
                          USB_HID_GENERIC_ENDPOINT_IN,
                          pData,
                          len);
        //LOGI(TAG, "USB_DeviceHidSend return : %u", ret);
    }
}

static void handleUSBRecvData(uint8_t *pData, uint16_t len)
{
    md_slave_msg_pack usbMsgPack = { 0x0, };
    usbMsgPack.mcv_Sender = MB_SENDER_USB;
    usbMsgPack.mcp_RespBuff = g_UsbDeviceHidGeneric.sendBuffer;
    usbMsgPack.resp_func = usb_send_data;
    usbMsgPack.mcp_ReceiveBuff = pData;
    usbMsgPack.msv_ReceiveLen = len;
    if(usbMsgPack.mcp_ReceiveBuff[0] == 0x00)
    {
        usbMsgPack.mcv_IsBroadcastInfo = 1;
    }

    //if((usbMsgPack.mcp_ReceiveBuff[0] == gtp_ModbusSlaveDiagInfo[usbMsgPack.mcv_Sender].mcv_SlaveId)|| usbMsgPack.mcv_IsBroadcastInfo)
    {
        gtp_ModbusSlaveDiagInfo[usbMsgPack.mcv_Sender].msv_SlavePackageCnt++;
        mb_slave_msg_handler(&usbMsgPack);
    }
}

/* The hid class callback */
static usb_status_t USB_DeviceHidGenericCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_endpoint_callback_message_struct_t *pMessage;
    usb_status_t error = kStatus_USB_Error;
    //LOGV(TAG, "Enter %s(), handle = 0x%08X, event = %X", __func__, handle, event);
    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse://Send finished, so we can send next
            gIfCanSend = true;
            pMessage = (usb_device_endpoint_callback_message_struct_t *)param;
            //LOGV(TAG, "pMessage->buffer = 0x%08X, pMessage->length = %u, pMessage->isSetup = %u", pMessage->buffer, pMessage->length, pMessage->isSetup);
        #if 0
            USB_DeviceHidSend(g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_IN,
                                  g_UsbDeviceHidGeneric.sendBuffer,
                                  USB_HID_GENERIC_OUT_BUFFER_LENGTH);
        #endif
            break;
        case kUSB_DeviceHidEventRecvResponse:// Receive finished, so we can receive next
            //LOGD(TAG, "g_UsbDeviceHidGeneric.attach = %u", g_UsbDeviceHidGeneric.attach);
            if (g_UsbDeviceHidGeneric.attach)
            {
                pMessage = (usb_device_endpoint_callback_message_struct_t *)param;
                //LOGD(TAG, "pMessage->buffer = 0x%08X, pMessage->length = %u, pMessage->isSetup = %u", pMessage->buffer, pMessage->length, pMessage->isSetup);
                //hexdump(pMessage->buffer, pMessage->length);
                //hexdump(pMessage->buffer, 64);
                //hexdump(g_UsbDeviceHidGeneric.recvBuffer, 64);
                if (pMessage->length > 1024)
                {
                    break;
                }
                handleUSBRecvData(pMessage->buffer, pMessage->length);
                #if 0
                USB_DeviceHidSend(g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_IN,
                                  (uint8_t *)&g_UsbDeviceHidGeneric.buffer[g_UsbDeviceHidGeneric.bufferIndex][0],
                                  USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                g_UsbDeviceHidGeneric.bufferIndex ^= 1U;
                #else
                
                #endif
                
#if 0
                return USB_DeviceHidRecv(g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                                         (uint8_t *)&g_UsbDeviceHidGeneric.buffer[g_UsbDeviceHidGeneric.bufferIndex][0],
                                         USB_HID_GENERIC_OUT_BUFFER_LENGTH);
#else
                return USB_DeviceHidRecv(g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                                         g_UsbDeviceHidGeneric.recvBuffer,
                                         USB_HID_GENERIC_OUT_BUFFER_LENGTH);
#endif
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

/* The device callback */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    uint8_t *temp8 = (uint8_t *)param;
    uint16_t *temp16 = (uint16_t *)param;
    LOGV(TAG, "Enter %s(), handle = 0x%08X, event = %u, param = 0x%08X", __func__, handle, event, param);
    LOGD(TAG, "%s: *temp8 = 0x%X, *temp16 = 0x%X", __func__, *temp8, *temp16);
    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* USB bus reset signal detected */
            g_UsbDeviceHidGeneric.attach = 0U;
            g_UsbDeviceHidGeneric.currentConfiguration = 0U;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_UsbDeviceHidGeneric.speed))
            {
                LOGI(TAG, "%s: speed = %u", __func__, g_UsbDeviceHidGeneric.speed);
                USB_DeviceSetSpeed(handle, g_UsbDeviceHidGeneric.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_UsbDeviceHidGeneric.attach = 0U;
                g_UsbDeviceHidGeneric.currentConfiguration = 0U;
            }
            else if (USB_HID_GENERIC_CONFIGURE_INDEX == (*temp8))
            {
                LOGW(TAG, "%s: g_UsbDeviceHidGeneric.speed = %u", __func__, g_UsbDeviceHidGeneric.speed);
                /* Set device configuration request */
                g_UsbDeviceHidGeneric.attach = 1U;
                g_UsbDeviceHidGeneric.currentConfiguration = *temp8;
                #if 0
                error = USB_DeviceHidRecv(
                    g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                    (uint8_t *)&g_UsbDeviceHidGeneric.buffer[g_UsbDeviceHidGeneric.bufferIndex][0],
                    USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                #else
                error = USB_DeviceHidRecv(
                    g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                    g_UsbDeviceHidGeneric.recvBuffer,
                    USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                #endif
                gIfCanSend = true;
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_UsbDeviceHidGeneric.attach)
            {
                /* Set device interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_HID_GENERIC_INTERFACE_COUNT)
                {
                    g_UsbDeviceHidGeneric.currentInterfaceAlternateSetting[interface] = alternateSetting;
                    if (alternateSetting == 0U)
                    {
                        #if 0
                        error = USB_DeviceHidRecv(
                            g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                            (uint8_t *)&g_UsbDeviceHidGeneric.buffer[g_UsbDeviceHidGeneric.bufferIndex][0],
                            USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                        #else
                        error = USB_DeviceHidRecv(
                            g_UsbDeviceHidGeneric.hidHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                            g_UsbDeviceHidGeneric.recvBuffer,
                            USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                        #endif
                    }
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get current configuration request */
                *temp8 = g_UsbDeviceHidGeneric.currentConfiguration;
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_HID_GENERIC_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_UsbDeviceHidGeneric.currentInterfaceAlternateSetting[interface];
                    error = kStatus_USB_Success;
                }
                else
                {
                    error = kStatus_USB_InvalidRequest;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidDescriptor:
            if (param)
            {
                /* Get hid descriptor request */
                error = USB_DeviceGetHidDescriptor(handle, (usb_device_get_hid_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidReportDescriptor:
            if (param)
            {
                /* Get hid report descriptor request */
                error =
                    USB_DeviceGetHidReportDescriptor(handle, (usb_device_get_hid_report_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidPhysicalDescriptor:
            if (param)
            {
                /* Get hid physical descriptor request */
                error = USB_DeviceGetHidPhysicalDescriptor(handle,
                                                           (usb_device_get_hid_physical_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }
    PRINTF("\r\n");
    return error;
}

static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
asdf
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Set HID generic to default state */
    g_UsbDeviceHidGeneric.speed = USB_SPEED_FULL;
    g_UsbDeviceHidGeneric.attach = 0U;
    g_UsbDeviceHidGeneric.hidHandle = (class_handle_t)NULL;
    g_UsbDeviceHidGeneric.deviceHandle = NULL;
    #if 0
    g_UsbDeviceHidGeneric.buffer[0] = (uint8_t *)&s_GenericBuffer0[0];
    g_UsbDeviceHidGeneric.buffer[1] = (uint8_t *)&s_GenericBuffer1[0];
    #else
    g_UsbDeviceHidGeneric.recvBuffer = s_GenericBuffer0;
    g_UsbDeviceHidGeneric.sendBuffer = s_GenericBuffer1;
    #endif
    /* Initialize the usb stack and class drivers */
    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceHidConfigList, &g_UsbDeviceHidGeneric.deviceHandle))
    {
        LOGI(TAG, "USB device HID generic failed");
        return;
    }
    else
    {
        LOGI(TAG, "USB device HID generic demo");
        /* Get the HID mouse class handle */
        g_UsbDeviceHidGeneric.hidHandle = g_UsbDeviceHidConfigList.config->classHandle;
        LOGW(TAG, "g_UsbDeviceHidGeneric.deviceHandle = 0x%08X, g_UsbDeviceHidGeneric.hidHandle = 0x%08X", g_UsbDeviceHidGeneric.deviceHandle, g_UsbDeviceHidGeneric.hidHandle);
    }
    LOGV(TAG, "g_UsbDeviceHidGeneric.recvBuffer = 0x%08X", g_UsbDeviceHidGeneric.recvBuffer);
    LOGV(TAG, "g_UsbDeviceHidGeneric.sendBuffer = 0x%08X", g_UsbDeviceHidGeneric.sendBuffer);
    USB_DeviceIsrEnable();

    /* Start USB device HID generic */
    USB_DeviceRun(g_UsbDeviceHidGeneric.deviceHandle);
}

#if (USB_DEVICE_CONFIG_USE_TASK == 1)
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceEhciTaskFunction(deviceHandle);
}

void USB_DeviceTask(void *handle)
{
    LOGV(TAG, "%s RUN. Free heap size is %d bytes.", __func__, xPortGetFreeHeapSize());
    while (1)
    {
        USB_DeviceTaskFn(handle);
        //LOGD(TAG, "After run USB_DeviceTaskFn()");
    }
}
#endif

static bool ifMeSupportUSB(void)
{
    bool gate = bspIsGateway();
    uint16_t dID = bsp_get_deviceID();
    if (gate == true || dID == 0xFFFF)
    {
        return true;
    }
    return false;
}

void usb_task(void *p_arg)
{
    LOGV(TAG, "%s RUN, Free heap size is %d bytes.", __func__, xPortGetFreeHeapSize());
    if (ifMeSupportUSB() == false)
    {
        LOGE(TAG, "I do not support USB, so just return.");
        vTaskDelay(1000);
        vTaskDelete(NULL);
        return;
    }
    USB_DeviceApplicationInit();
#if (USB_DEVICE_CONFIG_USE_TASK == 1)
    if (g_UsbDeviceHidGeneric.deviceHandle)
    {
        if (xTaskCreate(USB_DeviceTask,                         /* pointer to the task */
                        "usb_device_task",                      /* task name for kernel awareness debugging */
                        USB_DEVICE_TASK_STACK_SIZE,             /* task stack size */
                        g_UsbDeviceHidGeneric.deviceHandle,     /* optional task startup argument */
                        USB_DEVICE_TASK_PRIORITY,               /* initial priority */
                        &g_UsbDeviceHidGeneric.deviceTaskHandle /* optional task handle to create */
                        ) != pdPASS)
        {
            LOGE(TAG, "usb device task create failed!");
            return;
        }
    }
    else
    {
        LOGE(TAG, "Init USB ERROR!");
    }
#else
    if (g_UsbDeviceHidGeneric.deviceHandle == NULL)
    {
        LOGE(TAG, "Init USB ERROR!!!");
        gUSBTaskHandler = NULL;
        vTaskDelete(NULL);
    }
#endif
    LOGI(TAG, "Init USB OK");
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //sprintf(sendData, "APP_task: send some data(%u)", counts++);
        //usb_send_data((uint8_t *)sendData, strlen(sendData));
        LOGV(TAG, "usb_task is running...g_UsbDeviceHidGeneric.deviceHandle = 0x%08X", g_UsbDeviceHidGeneric.deviceHandle);
        vTaskDelete(NULL);
    }
}

