
#ifndef _PLATFORM_H_
#define _PLATFORM_H_
#include <stdint.h>
#include <string.h>

#include "app_scheduler.h"
#define platform_simple_evt_put(handler) APP_ERROR_CHECK(app_sched_event_put(NULL,0,(app_sched_event_handler_t)handler))
#define platform_evt_put(data,size,handler) APP_ERROR_CHECK(app_sched_event_put(data,size,(app_sched_event_handler_t)handler))

#define LOG_INIT(x) NRF_LOG_MODULE_REGISTER();

#if NRF_LOG_ENABLED==1
	#define LOG_HEX_RAW LOG_HEX_RAW_IMP
	#define LOG_RAW NRF_LOG_RAW_INFO
#else
	#define LOG_HEX_RAW(x...)
	#define LOG_RAW(x...)
#endif

void LOG_HEX_RAW_IMP(const uint8_t* array, uint16_t length);

#define LED_PIN (22)

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "uevent.h"

void platform_init(void);
void platform_scheduler(void);

void platform_reboot(void);
void platform_powerdown(bool flag);

void random_refresh(void);
uint32_t platform_get_systick(void);

#define UEVT_RTC_BASE (0x0000)
#define UEVT_RTC_8HZ (0x0001)
#define UEVT_RTC_1HZ (0x0002)
#define UEVT_RTC_NEWDAY (0x0003)

#endif
