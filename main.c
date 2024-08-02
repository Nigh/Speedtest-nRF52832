
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "uevent.h"

#include "led_drv.h"

#include "app_timer.h"
#include "bluetooth.h"

#include "bt_air.h"

#ifdef CONFIG_NFCT_PINS_AS_GPIOS
	volatile uint32_t UICR_ADDR_0x20C __attribute__((section("uicr_nfc"))) = 0xFFFFFFFE;
#endif

bool is_bt_connect = false;

#define LOG_EVT(EVT) case EVT: LOG_RAW(#EVT "\n"); break;
const uint8_t led_blink[2] = {1, 1};
void log_on_uevt_handler(uevt_t* evt)
{
	static uint32_t sec = 0;
	switch(evt->evt_id) {
			LOG_EVT(UEVT_BT_INIT);
			LOG_EVT(UEVT_BT_ADV_START);
		case UEVT_RTC_1HZ:
			LOG_RAW("\n[%06d]:", sec);
			sec += 1;
			break;
		case UEVT_RTC_8HZ:
			break;
	}
}

void shutdown_now(void)
{
	app_timer_stop_all();
	platform_powerdown(true);
}

#include "nrf_gpio.h"

void main_handler(uevt_t* evt)
{
	switch(evt->evt_id) {
		case UEVT_RTC_1HZ:
			break;
		case UEVT_RTC_8HZ:
			break;
		case UEVT_BT_DATARECV:
			bt_air_interface(((ble_user_data_t*)(evt->content))->p_data, ((ble_user_data_t*)(evt->content))->p_length);
			break;
		case UEVT_BT_CONN:
			is_bt_connect = true;
			break;
		case UEVT_BT_DISCONN:
			is_bt_connect = false;
			break;
	}
}

void rtc_1hz_handler(void)
{
	uevt_bc_e(UEVT_RTC_1HZ);
}

void rtc_8hz_isr(uint8_t tick)
{
	uevt_bc_e(UEVT_RTC_8HZ);
}

void user_init(void)
{
	app_timer_init();
	user_event_handler_regist(log_on_uevt_handler);
	user_event_handler_regist(main_handler);
	bt_air_init();
}

int main(void)
{
	platform_init();
	user_init();

	LOG_RAW("RTT Started.\n");

	for (;;) {
		platform_scheduler();
	}
}


#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_util_platform.h"
#include "nrf_strerror.h"

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
	#include "nrf_sdm.h"
#endif
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	__disable_irq();
	NRF_LOG_FINAL_FLUSH();

#ifndef DEBUG
	NRF_LOG_ERROR("Fatal error");
#else
	switch (id) {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
		case NRF_FAULT_ID_SD_ASSERT:
			NRF_LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
			break;
		case NRF_FAULT_ID_APP_MEMACC:
			NRF_LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
			break;
#endif
		case NRF_FAULT_ID_SDK_ASSERT: {
			assert_info_t* p_info = (assert_info_t*)info;
			NRF_LOG_ERROR("ASSERTION FAILED at %s:%u",
			              p_info->p_file_name,
			              p_info->line_num);
			break;
		}
		case NRF_FAULT_ID_SDK_ERROR: {
			error_info_t* p_info = (error_info_t*)info;
			NRF_LOG_ERROR("ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x",
			              p_info->err_code,
			              nrf_strerror_get(p_info->err_code),
			              p_info->p_file_name,
			              p_info->line_num,
			              pc);
			NRF_LOG_ERROR("End of error report");
			break;
		}
		default:
			NRF_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
			break;
	}
#endif

	// On assert, the system can only recover with a reset.

#ifndef DEBUG
	NRF_LOG_WARNING("System reset");
	NVIC_SystemReset();
#else
	app_error_save_and_stop(id, pc, info);
#endif // DEBUG
	NRF_BREAKPOINT_COND;
}
