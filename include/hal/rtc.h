#ifndef HAL_RTC_H_INCLUDED
#define HAL_RTC_H_INCLUDED


#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_gcc.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif


struct hal_rtc_timer {
	void (*cb_fun)(void *arg);
	void *arg;
    ListItem_t lnode;
};


void hal_rtc_init(void);
void hal_rtc_deinit(void);

uint32_t hal_rtc_time(void);

bool hal_rtc_start(struct hal_rtc_timer *timer, uint32_t ticks);
bool hal_rtc_start_at(struct hal_rtc_timer *timer, uint32_t tick);
void hal_rtc_stop(struct hal_rtc_timer *timer);


#ifdef __cplusplus
}
#endif
#endif /*HAL_RTC_H_INCLUDED*/
