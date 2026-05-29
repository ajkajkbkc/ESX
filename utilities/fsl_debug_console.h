/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Debug console shall provide input and output functions to scan and print formatted data.
 * o Support a format specifier for PRINTF follows this prototype "%[flags][width][.precision][length]specifier"
 *   - [flags] :'-', '+', '#', ' ', '0'
 *   - [width]:  number (0,1...)
 *   - [.precision]: number (0,1...)
 *   - [length]: do not support
 *   - [specifier]: 'd', 'i', 'f', 'F', 'x', 'X', 'o', 'p', 'u', 'c', 's', 'n'
 * o Support a format specifier for SCANF follows this prototype " %[*][width][length]specifier"
 *   - [*]: is supported.
 *   - [width]: number (0,1...)
 *   - [length]: 'h', 'hh', 'l','ll','L'. ignore ('j','z','t')
 *   - [specifier]: 'd', 'i', 'u', 'f', 'F', 'e', 'E', 'g', 'G', 'a', 'A', 'o', 'c', 's'
 */

#ifndef _FSL_DEBUGCONSOLE_H_
#define _FSL_DEBUGCONSOLE_H_

#include "fsl_common.h"
#include "serial_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "kalyke_opts.h"

/*!
 * @addtogroup debugconsole
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

extern serial_handle_t g_serialHandle; /*!< serial manager handle */

/*! @brief Definition select redirect toolchain printf, scanf to uart or not. */
#define DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN 0U /*!< Select toolchain printf and scanf. */
#define DEBUGCONSOLE_REDIRECT_TO_SDK       1U /*!< Select SDK version printf, scanf. */
#define DEBUGCONSOLE_DISABLE               2U /*!< Disable debugconsole function. */

/*! @brief Definition to select sdk or toolchain printf, scanf. The macro only support
 * to be redefined in project setting.
 */
#ifndef SDK_DEBUGCONSOLE
    #if (LOG_OPEN == 1)
        #if (LOG_OPEN_CONSOLE == 1)
        #define SDK_DEBUGCONSOLE    DEBUGCONSOLE_REDIRECT_TO_SDK
        #else
        #define SDK_DEBUGCONSOLE    DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN
        #endif
    #else
        #define SDK_DEBUGCONSOLE    DEBUGCONSOLE_DISABLE
    #endif
#endif

#if defined(SDK_DEBUGCONSOLE) && !(SDK_DEBUGCONSOLE)
#include <stdio.h>
#endif

extern volatile TickType_t xTickCount;

/*! @brief Definition to select redirect toolchain printf, scanf to uart or not.
 *
 *  if SDK_DEBUGCONSOLE defined to 0,it represents select toolchain printf, scanf.
 *  if SDK_DEBUGCONSOLE defined to 1,it represents select SDK version printf, scanf.
 *  if SDK_DEBUGCONSOLE defined to 2,it represents disable debugconsole function.
 */
#if SDK_DEBUGCONSOLE == 2 /* Disable debug console */
#define PRINTF(...) \
    do              \
    {               \
    } while (0)
#define SCANF(...) \
    do             \
    {              \
    } while (0)
#define PUTCHAR(...) \
    do               \
    {                \
    } while (0)
#define GETCHAR() -1
#define LOGE(...)
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#define LOGV(...)
#define LOG_LWIP(...)

#elif SDK_DEBUGCONSOLE == 1 /* Select printf, scanf, putchar, getchar of SDK version. */
#if 1
#define PRINTF  DbgConsole_Printf
#define SCANF   DbgConsole_Scanf
#define PUTCHAR DbgConsole_Putchar
#define GETCHAR DbgConsole_Getchar
#else
#define PRINTF(...)
#define SCANF(...)
#define PUTCHAR(...)
#define GETCHAR(...)
#endif
        
typedef enum 
{
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;
        
#define LOG_LOCAL_LEVEL  5
        
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
        
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_BLUE)
#define LOG_COLOR_V       LOG_BOLD(LOG_COLOR_BLACK)
        
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%d) %s: " format LOG_RESET_COLOR "\r\n"
#if 1
#define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { DbgConsole_Printf(LOG_FORMAT(E, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)   { DbgConsole_Printf(LOG_FORMAT(W, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)   { DbgConsole_Printf(LOG_FORMAT(I, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { DbgConsole_Printf(LOG_FORMAT(D, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE)   { DbgConsole_Printf(LOG_FORMAT(V, format), xTickCount, tag, ##__VA_ARGS__); }
#else
#define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { DbgConsole_BlockingPrintf(LOG_FORMAT(E, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)   { DbgConsole_BlockingPrintf(LOG_FORMAT(W, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)   { DbgConsole_BlockingPrintf(LOG_FORMAT(I, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { DbgConsole_BlockingPrintf(LOG_FORMAT(D, format), xTickCount, tag, ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE)   { DbgConsole_BlockingPrintf(LOG_FORMAT(V, format), xTickCount, tag, ##__VA_ARGS__); }
#endif
#define LOG_FORMAT_LWIP(letter, format)  LOG_COLOR_ ## letter #letter " (%d) %s: " format LOG_RESET_COLOR
#define LOG_LWIP( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { DbgConsole_Printf(LOG_FORMAT_LWIP(D, format), xTaskGetTickCount(), tag, ##__VA_ARGS__); }    
#elif SDK_DEBUGCONSOLE == 0 /* Select printf, scanf, putchar, getchar of toolchain. */
#define LOGE(...)
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#define LOGV(...)
        
#define PRINTF  printf
#define SCANF   scanf
#define PUTCHAR putchar
#define GETCHAR getchar
#define LOG_LWIP(...)
    
#endif /* SDK_DEBUGCONSOLE */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*! @name Initialization*/
/* @{ */

#if ((SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK) || defined(SDK_DEBUGCONSOLE_UART))
/*!
 * @brief Initializes the peripheral used for debug messages.
 *
 * Call this function to enable debug log messages to be output via the specified peripheral
 * initialized by the serial manager module.
 * After this function has returned, stdout and stdin are connected to the selected peripheral.
 *
 * @param instance      The instance of the module.
 * @param baudRate      The desired baud rate in bits per second.
 * @param device        Low level device type for the debug console, can be one of the following.
 *                      @arg kSerialPort_Uart,
 *                      @arg kSerialPort_UsbCdc
 *                      @arg kSerialPort_UsbCdcVirtual.
 * @param clkSrcFreq    Frequency of peripheral source clock.
 *
 * @return              Indicates whether initialization was successful or not.
 * @retval kStatus_Success          Execution successfully
 */
status_t DbgConsole_Init(uint8_t instance, uint32_t baudRate, serial_port_type_t device, uint32_t clkSrcFreq);

/*!
 * @brief De-initializes the peripheral used for debug messages.
 *
 * Call this function to disable debug log messages to be output via the specified peripheral
 * initialized by the serial manager module.
 *
 * @return Indicates whether de-initialization was successful or not.
 */
status_t DbgConsole_Deinit(void);
#else
/*!
 * Use an error to replace the DbgConsole_Init when SDK_DEBUGCONSOLE is not DEBUGCONSOLE_REDIRECT_TO_SDK and
 * SDK_DEBUGCONSOLE_UART is not defined.
 */
static inline status_t DbgConsole_Init(uint8_t instance,
                                       uint32_t baudRate,
                                       serial_port_type_t device,
                                       uint32_t clkSrcFreq)
{
    (void)instance;
    (void)baudRate;
    (void)device;
    (void)clkSrcFreq;
    return (status_t)kStatus_Fail;
}
/*!
 * Use an error to replace the DbgConsole_Deinit when SDK_DEBUGCONSOLE is not DEBUGCONSOLE_REDIRECT_TO_SDK and
 * SDK_DEBUGCONSOLE_UART is not defined.
 */
static inline status_t DbgConsole_Deinit(void)
{
    return (status_t)kStatus_Fail;
}
#endif /* ((SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK) || defined(SDK_DEBUGCONSOLE_UART)) */

#if SDK_DEBUGCONSOLE
/*!
 * @brief Writes formatted output to the standard output stream.
 *
 * Call this function to write a formatted output to the standard output stream.
 *
 * @param   formatString Format control string.
 * @return  Returns the number of characters printed or a negative value if an error occurs.
 */
int DbgConsole_Printf(const char *formatString, ...);

/*!
 * @brief Writes a character to stdout.
 *
 * Call this function to write a character to stdout.
 *
 * @param   ch Character to be written.
 * @return  Returns the character written.
 */
int DbgConsole_Putchar(int ch);

/*!
 * @brief Reads formatted data from the standard input stream.
 *
 * Call this function to read formatted data from the standard input stream.
 *
 * @note Due the limitation in the BM OSA environment (CPU is blocked in the function,
 * other tasks will not be scheduled), the function cannot be used when the
 * DEBUG_CONSOLE_TRANSFER_NON_BLOCKING is set in the BM OSA environment.
 * And an error is returned when the function called in this case. The suggestion
 * is that polling the non-blocking function DbgConsole_TryGetchar to get the input char.
 *
 * @param   formatString Format control string.
 * @return  Returns the number of fields successfully converted and assigned.
 */
int DbgConsole_Scanf(char *formatString, ...);

/*!
 * @brief Reads a character from standard input.
 *
 * Call this function to read a character from standard input.
 *
 * @note Due the limitation in the BM OSA environment (CPU is blocked in the function,
 * other tasks will not be scheduled), the function cannot be used when the
 * DEBUG_CONSOLE_TRANSFER_NON_BLOCKING is set in the BM OSA environment.
 * And an error is returned when the function called in this case. The suggestion
 * is that polling the non-blocking function DbgConsole_TryGetchar to get the input char.
 *
 * @return Returns the character read.
 */
int DbgConsole_Getchar(void);

/*!
 * @brief Writes formatted output to the standard output stream with the blocking mode.
 *
 * Call this function to write a formatted output to the standard output stream with the blocking mode.
 * The function will send data with blocking mode no matter the DEBUG_CONSOLE_TRANSFER_NON_BLOCKING set
 * or not.
 * The function could be used in system ISR mode with DEBUG_CONSOLE_TRANSFER_NON_BLOCKING set.
 *
 * @param   formatString Format control string.
 * @return  Returns the number of characters printed or a negative value if an error occurs.
 */
int DbgConsole_BlockingPrintf(const char *formatString, ...);

/*!
 * @brief Debug console flush.
 *
 * Call this function to wait the tx buffer empty.
 * If interrupt transfer is using, make sure the global IRQ is enable before call this function
 * This function should be called when
 * 1, before enter power down mode
 * 2, log is required to print to terminal immediately
 * @return Indicates whether wait idle was successful or not.
 */
status_t DbgConsole_Flush(void);

#ifdef DEBUG_CONSOLE_TRANSFER_NON_BLOCKING
/*!
 * @brief Debug console try to get char
 * This function provides a API which will not block current task, if character is
 * available return it, otherwise return fail.
 * @param ch the address of char to receive
 * @return Indicates get char was successful or not.
 */
status_t DbgConsole_TryGetchar(char *ch);
#endif

#endif /* SDK_DEBUGCONSOLE */

/*! @} */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*! @} */

#endif /* _FSL_DEBUGCONSOLE_H_ */
