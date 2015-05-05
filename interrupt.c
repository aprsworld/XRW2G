#int_timer1
void isr_10ms(void) {
	static int16 uptimeticks=0;
	static int16 adcTicks=0;
	static int8 ticks=0;

	clear_interrupt(INT_TIMER1);
	set_timer1(45536);

	output_high(PIN_D5);

	/* anemometers quit moving */
	if ( 0xffff == timers.pulse_period[0] )
				current.pulse_period[0]=0;
	if ( 0xffff == timers.pulse_period[1] )
				current.pulse_period[1]=0;
	if ( 0xffff == timers.pulse_period[2] )
				current.pulse_period[2]=0;

	/* seconds since last query */
	if ( current.interval_milliseconds < 65535 ) {
		current.interval_milliseconds++;
	}
	if ( strobed_data.interval_milliseconds < 65535 ) {
		strobed_data.interval_milliseconds++;
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

#int_timer2
void isr_100us(void) {
	if ( bit_test(timers.pulse_ch_en,0) && 0xffff != timers.pulse_period[0] )
		timers.pulse_period[0]++;
	if ( bit_test(timers.pulse_ch_en,1) && 0xffff != timers.pulse_period[1] )
		timers.pulse_period[1]++;
	if ( bit_test(timers.pulse_ch_en,2) && 0xffff != timers.pulse_period[2] )
		timers.pulse_period[2]++;
}

#int_ext
/* high resolution pulse timer / counter triggered on rising edge */
void isr_ext0(void) {
	static short state=0;
	current.pulse_count[0]++;
	current.pulse_sum[0]++;

	if ( 1 == state ) {
		/* currently counting, time to finish */
		bit_clear(timers.pulse_ch_en,0);
		current.pulse_period[0]=timers.pulse_period[0];
		if ( current.pulse_period[0] < current.pulse_min_period[0] ) {
			current.pulse_min_period[0]=current.pulse_period[0];
		}
		if ( current.pulse_period[0] > current.pulse_max_period[0] ) {
			current.pulse_max_period[0]=current.pulse_period[0];
		}
		state=0;
	}

	if ( 0 == state ) {
		/* not counting, time to start */
		timers.pulse_period[0]=0;
		bit_set(timers.pulse_ch_en,0);
		state=1;
	}
}

#int_ext1
void isr_ext1(void) {
	static short state=0;
	current.pulse_count[1]++;
	current.pulse_sum[1]++;

	if ( 1 == state ) {
		/* currently counting, time to finish */
		bit_clear(timers.pulse_ch_en,1);
		current.pulse_period[1]=timers.pulse_period[1];
		if ( current.pulse_period[1] < current.pulse_min_period[1] ) {
			current.pulse_min_period[1]=current.pulse_period[1];
		}
		if ( current.pulse_period[1] > current.pulse_max_period[1] ) {
			current.pulse_max_period[1]=current.pulse_period[1];
		}
		state=0;
	}

	if ( 0 == state ) {
		/* not counting, time to start */
		timers.pulse_period[1]=0;
		bit_set(timers.pulse_ch_en,1);
		state=1;
	}
}

#int_ext2
void isr_ext2(void) {
	static short state=0;
	current.pulse_count[2]++;
	current.pulse_sum[2]++;

	if ( 1 == state ) {
		/* currently counting, time to finish */
		bit_clear(timers.pulse_ch_en,2);
		current.pulse_period[2]=timers.pulse_period[2];
		if ( current.pulse_period[2] < current.pulse_min_period[2] ) {
			current.pulse_min_period[2]=current.pulse_period[2];
		}
		if ( current.pulse_period[2] > current.pulse_max_period[2] ) {
			current.pulse_max_period[2]=current.pulse_period[2];
		}
		state=0;
	}

	if ( 0 == state ) {
		/* not counting, time to start */
		timers.pulse_period[2]=0;
		bit_set(timers.pulse_ch_en,2);
		state=1;
	}
}
#int_rb
void isr_rb(void) {
	static int8 last_b=0xff;
	int8 b;

	/* current port b must be read before interrupt will quit firing */
	b=PORTB;

	/* look for rising edge on SYNC in (B4) */
	if ( ! bit_test(last_b,4) && bit_test(b,4) ) {
		/* got a sync pulse. Schedule a live packet and restart the seconds counter */
		timers.now_live_send=1;
		timers.live_seconds=0;
	}

	last_b=b;
}
