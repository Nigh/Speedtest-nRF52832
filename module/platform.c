
#define NRF_LOG_MODULE_NAME platform
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "platform.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"

#include "app_button.h"
#include "app_timer.h"

#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_pwr_mgmt.h"

#include "app_scheduler.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"

#include "nrf_queue.h"
#include "nrf_drv_rng.h"
#include "nrfx_rng.h"

#include "bluetooth.h"

#include <stdio.h>
void LOG_HEX_RAW_IMP(const uint8_t* array, uint16_t length)
{
	static char buffer[193];
	const uint8_t* pa = array;
	char* pb;
	uint8_t len;	// 当次处理长度
	while(length > 0) {
		pb = buffer;
		if(length > 64) {
			len = 64;
		} else {
			len = length;
		}
		for (uint8_t i = 0; i < len; i++) {
			sprintf(pb, "%02X ", *pa);
			pb += 3;
			pa += 1;
		}
		length -= len;
		*pb = 0;
		LOG_RAW("%s\n", buffer);
	}
}

void io_config(void);

#define USE_FPU
#ifdef USE_FPU
#define FPU_EXCEPTION_MASK 0x0000009F
void FPU_IRQHandler(void)
{
	uint32_t* fpscr = (uint32_t*)(FPU->FPCAR + 0x40);
	(void)__get_FPSCR();
	*fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}
#endif

void delay_ms(uint16_t ms)
{
	nrf_delay_ms(ms);
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
uint8_t grands_array[32];
void random_seed_init(void)
{
	APP_ERROR_CHECK(nrf_drv_rng_init(NULL));
}

void random_refresh(void)
{
	uint32_t err_code;
	uint8_t  available;

	nrf_drv_rng_bytes_available(&available);
	uint8_t length = MIN(32, available);

	err_code = nrf_drv_rng_rand(grands_array, length);
	APP_ERROR_CHECK(err_code);
}

__WEAK void rtc_8hz_isr(uint8_t tick)
{
	LOG_RAW("(WEAK HANDLER)isr - %d\r\n", tick);
}

__WEAK void rtc_1hz_handler(void)
{
	static uint32_t sec = 0;
	LOG_RAW("(WEAK HANDLER)tick - %d\r\n", sec);
	sec += 1;
}

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
	static unsigned char tick = 0;
	if (int_type == NRF_DRV_RTC_INT_COMPARE0) {
	} else if (int_type == NRF_DRV_RTC_INT_TICK) {
		rtc_8hz_isr(tick);
		tick += 1;
		if(tick & 0x8) {
			tick &= 0x7;
			platform_simple_evt_put(rtc_1hz_handler);
		}
	}
}
static void lfclk_config(void)
{
	ret_code_t err_code = nrf_drv_clock_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_clock_lfclk_request(NULL);
}
static void rtc_config(void)
{
	uint32_t err_code;
	nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
	config.prescaler = RTC_FREQ_TO_PRESCALER(8);
	err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_rtc_tick_enable(&rtc, true);
	nrf_drv_rtc_enable(&rtc);
}
static void app_timer_config(void)
{
	app_timer_init();
}

uint32_t platform_get_systick(void)
{
	// uint32_t cur_tick = nrf_drv_rtc_counter_get(&rtc);
	return app_timer_cnt_get();
}

static void log_init(void)
{
	ret_code_t err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void power_management_init(void)
{
	ret_code_t err_code;
	err_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(err_code);
}

#define REG_POWER_BASE_ADDR	0x40000000
#define RESET_REASON_OFFSET	0x400

uint32_t read_chip_reset_reason(void)
{
	return *(uint32_t*)(REG_POWER_BASE_ADDR + RESET_REASON_OFFSET);
}

void leds_config(void)
{
	nrf_gpio_cfg_output(LED_PIN);
	nrf_gpio_pin_set(LED_PIN);
}
void led_on(void)
{
	nrf_gpio_pin_clear(LED_PIN);
}

void led_off(void)
{
	nrf_gpio_pin_set(LED_PIN);
}

void io_config(void)
{
	// leds_config();
}

void factory_error(uint8_t a)
{

}

bool is_factory_pass(void)
{
	return false;
}

void factory_check(void)
{
	platform_powerdown(true);
}

uint8_t self_check(void)
{
	uint8_t a = 0;
	return a;
}

void peripheral_init(void)
{
}

extern void user_init(void);

void platform_init(void)
{
#if NRF_LOG_ENABLED==1
	log_init();
#endif
	// Initialize the async SVCI interface to bootloader before any interrupts are enabled.
	// 在没有烧录bootloader时，必须禁用，否则会报错
	//uint32_t err_code = ble_dfu_buttonless_async_svci_init();
	//APP_ERROR_CHECK(err_code);

	LOG_RAW("RESET=0x%04X\r\n", read_chip_reset_reason());
	APP_SCHED_INIT(32, 32);
	user_event_handler_array_init();
	lfclk_config();
	power_management_init();

	bluetooth_init();
	uevt_bc_e(UEVT_BT_CTL_ADV_ON);

	random_seed_init();
	random_refresh();

	sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
	nrf_drv_gpiote_init();
	io_config();
	// adc_config();
	// pwm_config();
#ifdef USE_FPU
	NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_HIGH);
	NVIC_EnableIRQ(FPU_IRQn);
#endif
	// self_check();
	rtc_config();
	app_timer_config();

	peripheral_init();
}

void platform_reboot(void)
{
	sd_nvic_SystemReset();
}

bool is_going_to_shutdown = false;
void platform_powerdown(bool flag)
{
	is_going_to_shutdown = flag;
}

void shutdown_routine(void)
{
	// nrf_gpio_cfg_sense_set(BUTTON2_PIN, BUTTON_SENSE);
	// nrf_gpio_cfg_sense_set(BUTTON3_PIN, BUTTON_SENSE);
	nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
	while(1);
}

void platform_scheduler(void)
{
	app_sched_execute();
	if (NRF_LOG_PROCESS() == false) {
		if(is_going_to_shutdown) {
			shutdown_routine();
		}
		nrf_pwr_mgmt_run();
	}
}
