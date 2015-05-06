#include "XRW2G.h"


typedef struct {
	int8 modbus_address;
	int8 modbus_speed;
	int8 modbus_mode;

	int8 serial_prefix;
	int16 serial_number;

	int8 adc_average_mode[8];
	int16 adc_sample_ticks;

	int16 worldData_seconds;
	int8  worldData_trigger_prefix;
	int16 worldData_trigger_number;

	int8  io_enable;
	int8  io_mode[3];

	int16 modbus_strobe_address;
	int16 modbus_strobe_register;

	int8 sync_in_enable;
} struct_config;



typedef struct {
	/* most recent valid */
	int16 pulse_period[3];
	int16 pulse_min_period[3];
	int16 pulse_max_period[3];
	int16 pulse_count[3];
	int32 pulse_sum[3];

	int16 adc_std_dev[8];

	/* circular buffer for ADC readings */
	int16 adc_buffer[8][16];
	int8  adc_buffer_index;

	int16 modbus_our_packets;
	int16 modbus_other_packets;
	int16 modbus_last_error;

	int16 sequence_number;
	int16 uptime_minutes;
	int16 interval_10milliseconds;

	int8 factory_unlocked;

	int8 worldData_enabled;
} struct_current;

typedef struct {
	int16 pulse_period[4];
	int16 pulse_count[4];
	int8 pulse_ch_en;

	int8 led_on_green;
	int8 led_on_red;

	int1 now_adc_sample;
	int1 now_adc_reset_count;
	int1 now_modbus_speed;
	int1 now_live_send;
	int1 now_10millisecond;

	int16 live_seconds;

} struct_time_keep;

typedef struct {
	int16 strobed_data[43];
	int16 interval_10milliseconds;
} struct_strobed_data;

/* global structures */
struct_config config;
struct_current current;
struct_time_keep timers;
struct_strobed_data strobed_data;


#include "adc_xrw2g.c"
#include "interrupt_xrw2g.c"
#include "param_xrw2g.c"

#include "modbus_slave_XRW2G.c"
#include "modbus_handler_XRW2G.c"

#include "live_xrw2g.c"




void init() {
	int8 i;

	setup_oscillator(OSC_8MHZ|OSC_INTRC);

	setup_adc_ports(AN0_TO_AN7 | VSS_VREF);
	setup_adc(ADC_CLOCK_INTERNAL);


	/* data structure initialization */
	timers.led_on_green=0;
	timers.led_on_red=0;
	timers.now_adc_sample=0;
	timers.now_adc_reset_count=0;
	timers.now_modbus_speed=0;
	timers.now_live_send=0;
	timers.now_10millisecond=0;
	timers.live_seconds=0;

	for ( i=0 ; i<3 ; i++ ) {
		current.pulse_period[i]=0;
		current.pulse_min_period[i]=65535;
		current.pulse_max_period[i]=0;
		current.pulse_count[i]=0;
		current.pulse_sum[i]=0;
	}
	current.modbus_our_packets=0;
	current.modbus_other_packets=0;
	current.modbus_last_error=0;
	current.sequence_number=0;
	current.uptime_minutes=0;
	current.interval_10milliseconds=0;
	current.adc_buffer_index=0;
	current.factory_unlocked=0;

	/* strobed data */
	memset(&strobed_data, 0x00, sizeof(strobed_data));
	strobed_data.interval_10milliseconds=65535;



	/* interrupts */

	/* one periodic interrupt @ 100uS. Generated from system 8 MHz clock */
	/* prescale=4, match=49, postscale=1. Match is 49 because when match occurs, one cycle is lost */
	setup_timer_2(T2_DIV_BY_4,49,1); 
	enable_interrupts(INT_TIMER2);

	output_low(XBEE_SLEEP);
	port_b_pullups(TRUE);
	delay_ms(14);
	current.worldData_enabled=!input(JP_WORLDDATA);

}

void periodic_10millisecond(void) {
	static int16 uptimeticks=0;
	static int16 adcTicks=0;
	static int8 ticks=0;



	/* anemometers quit moving */
	if ( 0xffff == timers.pulse_period[0] )
				current.pulse_period[0]=0;
	if ( 0xffff == timers.pulse_period[1] )
				current.pulse_period[1]=0;
	if ( 0xffff == timers.pulse_period[2] )
				current.pulse_period[2]=0;

	/* seconds since last query */
	if ( current.interval_10milliseconds < 65535 ) {
		output_high(PIN_D5);
		current.interval_10milliseconds++;
	}
	if ( strobed_data.interval_10milliseconds < 65535 ) {
		strobed_data.interval_10milliseconds++;
	}

	/* seconds */
	ticks++;
	if ( 100 == ticks ) {
		ticks=0;

		timers.live_seconds++;
		if ( timers.live_seconds >= config.worldData_seconds ) {
			timers.now_live_send=1;
			timers.live_seconds=0;
		}
	}

	/* uptime counter */
	uptimeTicks++;
	if ( 6000 == uptimeTicks ) {
		uptimeTicks=0;
		if ( current.uptime_minutes < 65535 ) 
			current.uptime_minutes++;
	}

	/* ADC sample counter */
	if ( timers.now_adc_reset_count ) {
		timers.now_adc_reset_count=0;
		adcTicks=0;
	}

	adcTicks++;
	if ( adcTicks == config.adc_sample_ticks ) {
		adcTicks=0;
		timers.now_adc_sample=1;
	}

	/* LEDs */
	if ( 0==timers.led_on_green ) {
		output_low(LED_GREEN);
	} else {
		output_high(LED_GREEN);
		timers.led_on_green--;
	}

	if ( 0==timers.led_on_red ) {
		output_low(LED_RED);
	} else {
		output_high(LED_RED);
		timers.led_on_red--;
	}

	output_low(PIN_D5);
}




void main(void) {
	int8 i;

	init();
	read_param_file();

	if ( config.modbus_address > 127 || config.modbus_speed > 1 ) {
		write_default_param_file();
	}

	/* interrupt on change handles SYNC_IN and GPS_PPS */
	if ( config.sync_in_enable ) {
		enable_interrupts(INT_RB);
	}

	/* LED test sequence */
	output_high(LED_RED);
	delay_ms(250);
	output_high(LED_GREEN);
	delay_ms(250);
	output_low(LED_RED);
	delay_ms(250);
	output_low(LED_GREEN);

#if 0
	/* FOR CTIW only */
	current.worldData_enabled=1;
	config.worldData_seconds=15;
#endif


	if ( current.worldData_enabled ) {
		/* in world data transmit mode */
		set_uart_speed(9600,MODBUS_SERIAL);
		enable_interrupts(INT_RDA);
		enable_interrupts(GLOBAL);
	} else {
		/* start out Modbus slave */
		setup_uart(TRUE);
		/* modbus_init turns on global interrupts */
		modbus_init();
		/* modbus initializes @ 9600 */
		if ( BAUD_19200 == config.modbus_speed )
			set_uart_speed(19200,MODBUS_SERIAL);
	}		

#if 0
	fprintf(MODBUS_SERIAL,"# XRW2G (%c%lu) WorldData Start Up (modbus speed=%u address=%u) %s\r\n",
		config.serial_prefix,
		config.serial_number,
		config.modbus_speed,
		config.modbus_address,
		__DATE__
	);
	fprintf(MODBUS_SERIAL,"# current.worldData_enabled=%u\r\n",current.worldData_enabled);
#endif

	/* Prime ADC filter */
	for ( i=0 ; i<30 ; i++ ) {
		adc_update();
	}

	for ( ; ; ) {
		restart_wdt();

		if ( timers.now_10millisecond ) {
			periodic_10millisecond();
			timers.now_10millisecond=0;
		}

		if ( timers.now_adc_sample ) {
			timers.now_adc_sample=0;
			adc_update();
		}

		if ( current.worldData_enabled ) {
			if ( timers.now_live_send ) {
				timers.now_live_send=0;
				live_send();
			}
		} else {
			modbus_process();

			if ( timers.now_modbus_speed ) {
				timers.now_modbus_speed=0;
				/* flip the speed */
				if ( BAUD_9600 == config.modbus_speed ) {
					set_uart_speed(19200,MODBUS_SERIAL);
					config.modbus_speed=BAUD_19200;
				} else {
					set_uart_speed(9600,MODBUS_SERIAL);
					config.modbus_speed=BAUD_9600;
				}
			}
		}
	}
}
