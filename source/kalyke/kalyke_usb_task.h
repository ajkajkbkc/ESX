/**
  ******************************************************************************
  * @file    kalyke_usb_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-07
  * @brief   USB task
  ******************************************************************************
  */

#ifndef __KALYKE_USB_TASK_H
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_hid.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif

#if defined(__GIC_PRIO_BITS)
#define USB_DEVICE_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_DEVICE_INTERRUPT_PRIORITY (5U)
#else
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#endif

typedef struct _usb_hid_generic_struct
{
    usb_device_handle deviceHandle;
    class_handle_t hidHandle;
    //TaskHandle_t applicationTaskHandle;
    TaskHandle_t deviceTaskHandle;
    //uint8_t *buffer[2];
    uint8_t *recvBuffer;
    uint8_t *sendBuffer;
    uint8_t bufferIndex;
    uint8_t idleRate;
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_HID_GENERIC_INTERFACE_COUNT];
} usb_hid_generic_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

extern TaskHandle_t gUSBTaskHandler;


extern void usb_task(void *p_arg);

#endif /* __KALYKE_USB_TASK_H */
