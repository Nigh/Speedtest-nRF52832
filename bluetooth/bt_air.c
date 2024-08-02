
#include "bt_air.h"
#include "nrf_log.h"
#include "bluetooth.h"
#include "build.h"


#define INT2CHAR(x) (x)&0xFF,(x)>>8
uint16_t ble_send_public_stack_depth = 0;
uint8_t ble_send_public_stack[256];
ble_user_data_t ble_send_public_data;
uint8_t gflowcontrol = 0;

#define ble_stack_init() ble_send_public_stack_depth=0
#define ble_stack_push(c) ble_send_public_stack[ble_send_public_stack_depth++]=c
#define ble_stack_push_uint16(u16) \
	ble_send_public_stack[ble_send_public_stack_depth++]=u16&0xFF;\
	ble_send_public_stack[ble_send_public_stack_depth++]=u16>>8
#define ble_stack_push_uint32(u32) \
	ble_send_public_stack[ble_send_public_stack_depth++]=u32&0xFF;\
	ble_send_public_stack[ble_send_public_stack_depth++]=(u32>>8)&0xFF;\
	ble_send_public_stack[ble_send_public_stack_depth++]=(u32>>16)&0xFF;\
	ble_send_public_stack[ble_send_public_stack_depth++]=(u32>>24)&0xFF
#define ble_stack_send() do{\
		ble_send_public_data.p_data=ble_send_public_stack;\
		ble_send_public_data.p_length=&ble_send_public_stack_depth;\
		uevt_bc(UEVT_BT_CTL_DATASEND, &ble_send_public_data);\
	}while(0)

#define ble_send(arr...) do{\
		const uint8_t t[]={arr};\
		const uint8_t len = sizeof(t);\
		ble_stack_init();\
		for (uint8_t i = 0; i < len; i++) { ble_stack_push(t[i]); }\
		ble_stack_send();\
		LOG_RAW("Tx:");\
		LOG_HEX_RAW(t, sizeof(t));\
	}while(0)

#define ack() ble_send(a[0],0)
#define err(x) ble_send(a[0],3,0xFF,INT2CHAR(x));LOG_RAW("ret ERR:%d\n",x)

#define ble_send_str(v1,v2,str) do{\
		const uint8_t t[]=str;\
		ble_stack_init();\
		ble_stack_push(a[0]);\
		ble_stack_push(sizeof(t)+2);\
		ble_stack_push(v1);\
		ble_stack_push(v2);\
		for (uint8_t i = 0; i < sizeof(t); i++) {\
			ble_stack_push(t[i]);\
		}\
		ble_stack_push(0);\
		ble_stack_send();\
		LOG_RAW("Tx:");\
		LOG_HEX_RAW(ble_send_public_stack, ble_send_public_stack_depth);\
	}while(0)

#include "app_timer.h"
extern void speed_test_package_init(void);
extern void speed_test_package_send_next(void);
void bt_air_interface(uint8_t* a, uint16_t* length)
{
	if(*length >= 4) {
		if(a[0] == 0x01 && a[1] == 0xAA && a[2] == 0x03 && a[3] == 0x00) {
			// speed test routine start
			speed_test_package_init();
			speed_test_package_send_next();
		}
	}
}

void bt_air_handler(uevt_t* evt)
{
	switch(evt->evt_id) {
		case UEVT_RTC_8HZ:
			break;
	}
}

void bt_air_init(void)
{
	user_event_handler_regist(bt_air_handler);
}
