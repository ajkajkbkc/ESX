/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#ifndef _osal_defs_
#define _osal_defs_

#include "fsl_debug_console.h"

#ifdef __cplusplus
extern "C"
{
#endif

// define if debug printf is needed
#define EC_DEBUG

#ifdef EC_DEBUG
    #if (LOG_OPEN == 1)
    #define tag "EtherCAT"
    //#define EC_PRINT(format, ...) DbgConsole_BlockingPrintf(LOG_FORMAT(D, format), xTickCount, tag, ##__VA_ARGS__);
    #define EC_PRINT(format, ...) DbgConsole_Printf(LOG_FORMAT(D, format), xTickCount, tag, ##__VA_ARGS__);
    #else
    #define EC_PRINT(...) do {} while (0)
    #endif
#else
#define EC_PRINT(...) do {} while (0)
#endif

#ifndef PACKED
#define PACKED_BEGIN
#define PACKED
#define PACKED_END
#endif

#define OSAL_THREAD_HANDLE task_t *
#define OSAL_THREAD_FUNC void
#define OSAL_THREAD_FUNC_RT void

#ifdef __cplusplus
}
#endif

#endif
