const int8 adcChannelMap[8]={0, 1, 2, 0, 4, 5, 7, 6};

int16 adc_get_olympic(int8 ch) {
	int16 min, max, sum;
	int16 l;
	int8 i;

	// Calculate the mean.  This is done by summing up the
	// values and dividing by the number of elements.
	min=65535;
	max=0;
	sum = 0;
	for( i = 0; i < 16 ; i++ ) {
		l=current.adc_buffer[ch][i];
		sum += l;

		if ( l > max )
			max=l;
		if ( l < min )
			min=l;
	}

	/* throw out the highest and lowest values */
	sum -= max;
	sum -= min;

	/* divide sum by our 14 samples and round by adding 7 */
	l=( (sum+7) / 14 );

	return l;
}

int16 adc_get(int8 ch) {
	int16 sum;
	int8 i;

	// Calculate the mean.  This is done by summing up the
	// values and dividing by the number of elements.
	sum = 0;
	for( i = 0; i < 16 ; i++ ) {
		sum += current.adc_buffer[ch][i];;
	}

	/* divide sum by our 16 samples and round by adding 8 */
	return ( (sum+8) >> 4 );
}


void adc_update(void) {
	int8 i;

	/* wrap buffer around */
	current.adc_buffer_index++;
	if ( current.adc_buffer_index >= 16 )
		current.adc_buffer_index=0;

	for ( i=0 ; i<8 ; i++ ) {
		/* input voltage and temperature are mux'ed on an0 */
		if ( 3==i ) {
			output_high(ADC_MUX_SEL);
		} else {
			output_low(ADC_MUX_SEL);
		}

		set_adc_channel(adcChannelMap[i]);
//		if ( 7==i) output_toggle(TP1);
		current.adc_buffer[i][current.adc_buffer_index] = read_adc();

//		if ( current.adc_buffer[i][current.adc_buffer_index] > 4095 )
//			timers.led_on_red=300;

//		current.adc[i]=mean_filter16_n( i, current.adc_last[i] );
		current.adc_std_dev[i]=0;
	}




}