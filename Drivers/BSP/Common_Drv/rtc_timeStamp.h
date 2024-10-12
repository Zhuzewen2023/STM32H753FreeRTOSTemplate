#ifndef __RTC_TIMESTAMP_H
#define __RTC_TIMESTAMP_H

#define RTC_CLOCK_SOURCE_LSI

#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV       0x7F //设置异步预分频器值128 - 1
#define RTC_SYNCH_PREDIV        0XF9 //设置同步预分频器值250 - 1
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
#define RTC_ASYNCH_PREDIV  0x7F
#define RTC_SYNCH_PREDIV   0x00FF
#endif

void rtcTimeStampInit(void);
void RTC_CalendarShow(void);

#endif
