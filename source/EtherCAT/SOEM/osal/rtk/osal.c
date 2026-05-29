/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include "FreeRTOS.h"
#include "task.h"

#include <osal.h>
//#include <kern.h>
#include <time.h>
//#include <sys/time.h>
//#include <config.h>
#include "bsp.h"
#include "fsl_debug_console.h"

#define  timercmp(a, b, CMP)                                \
  (((a)->sec == (b)->sec) ?                           \
   ((a)->usec CMP (b)->usec) :                        \
   ((a)->sec CMP (b)->sec))

#define  timeradd(a, b, result)                             \
  do {                                                      \
    (result)->sec = (a)->sec + (b)->sec;           \
    (result)->usec = (a)->usec + (b)->usec;        \
    if ((result)->usec >= 1000000)                       \
    {                                                       \
       ++(result)->sec;                                  \
       (result)->usec -= 1000000;                        \
    }                                                       \
  } while (0)

#if 0 // LiXianyu 20200801
#define  timersub(a, b, result)                             \
  do {                                                      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;           \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;        \
    if ((result)->tv_usec < 0) {                            \
      --(result)->tv_sec;                                   \
      (result)->tv_usec += 1000000;                         \
    }                                                       \
  } while (0)
#endif

#define USECS_PER_SEC   1000000

#if 0 // LiXianyu 20200801
#define USECS_PER_TICK  (USECS_PER_SEC / CFG_TICKS_PER_SECOND)
#endif

/* Workaround for rt-labs defect 776.
 * Default implementation of udelay() didn't work correctly when tick was
 * shorter than one millisecond.
 */
#if 0 // LiXianyu 20200801
void udelay (uint32_t us)
{
   tick_t ticks = (us / USECS_PER_TICK) + 1;
   task_delay (ticks);
}

int gettimeofday(struct timeval *tp, void *tzp)
{
   tick_t tick = tick_get();
   tick_t ticks_left;

   ASSERT (tp != NULL);

   tp->tv_sec = tick / CFG_TICKS_PER_SECOND;

   ticks_left = tick % CFG_TICKS_PER_SECOND;
   tp->tv_usec = ticks_left * USECS_PER_TICK;
   ASSERT (tp->tv_usec < USECS_PER_SEC);

   return 0;
}
#endif

#if 0 // LiXianyu 20200801
int osal_usleep (uint32 usec)
{
   udelay(usec);
   return 0;
}
#else
/* Task delay 0 only yields 
 * RT1061 do not support us delay.
*/
int osal_usleep (uint32 usec)
{
    vTaskDelay(usec / 1000);
    return 0;
}
#endif

#if 0 // LiXianyu 20200801
int osal_gettimeofday(struct timeval *tv, struct timezone *tz)
{
   return gettimeofday(tv, tz);
}
#endif

#if 0 // LiXianyu 20200801
ec_timet osal_current_time (void)
{
   struct timeval current_time;
   ec_timet return_value;

   gettimeofday (&current_time, 0);
   return_value.sec = current_time.tv_sec;
   return_value.usec = current_time.tv_usec;
   return return_value;
}
#else
ec_timet osal_current_time(void)
{
    ec_timet return_value;
    kalyke_getSystemTime(&return_value.sec, &return_value.usec);
    return return_value;
}
#endif

#if 0 // LiXianyu 20200801
void osal_timer_start (osal_timert * self, uint32 timeout_usec)
{
   struct timeval start_time;
   struct timeval timeout;
   struct timeval stop_time;

   gettimeofday (&start_time, 0);
   timeout.tv_sec = timeout_usec / USECS_PER_SEC;
   timeout.tv_usec = timeout_usec % USECS_PER_SEC;
   timeradd (&start_time, &timeout, &stop_time);

   self->stop_time.sec = stop_time.tv_sec;
   self->stop_time.usec = stop_time.tv_usec;
}
#else
void osal_timer_start(osal_timert * self, uint32 timeout_usec)
{
   ec_timet start_time;
   ec_timet timeout;
   ec_timet stop_time;

   kalyke_getSystemTime(&start_time.sec, &start_time.usec);
   timeout.sec  = timeout_usec / USECS_PER_SEC;
   timeout.usec = timeout_usec % USECS_PER_SEC;
   timeradd(&start_time, &timeout, &stop_time);

   self->stop_time.sec = stop_time.sec;
   self->stop_time.usec = stop_time.usec;
}
#endif

#if 0 // LiXianyu 20200801
boolean osal_timer_is_expired (osal_timert * self)
{
   struct timeval current_time;
   struct timeval stop_time;
   int is_not_yet_expired;

   gettimeofday (&current_time, 0);
   stop_time.tv_sec = self->stop_time.sec;
   stop_time.tv_usec = self->stop_time.usec;
   is_not_yet_expired = timercmp (&current_time, &stop_time, <);

   return is_not_yet_expired == false;
}
#else
boolean osal_timer_is_expired (osal_timert * self)
{
   ec_timet current_time;
   ec_timet stop_time;
   int is_not_yet_expired;

   //gettimeofday (&current_time, 0);
   kalyke_getSystemTime(&current_time.sec, &current_time.usec);
   stop_time.sec  = self->stop_time.sec;
   stop_time.usec = self->stop_time.usec;
   is_not_yet_expired = timercmp(&current_time, &stop_time, <);

   return is_not_yet_expired == false;
}
#endif

void *osal_malloc(size_t size)
{
   return pvPortMalloc(size);
}

void osal_free(void *ptr)
{
   vPortFree(ptr);
}

int osal_thread_create(void *thandle, int stacksize, void *func, void *param)
{
   
   return 1;
}

int osal_thread_create_rt(void *thandle, int stacksize, void *func, void *param)
{
   
   return 1;
}
