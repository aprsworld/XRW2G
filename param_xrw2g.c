#inline
char xor_crc(char oldcrc, char data) {
	return oldcrc ^ data;
}

char EEPROMDataRead( int16 address, int8 *data, int16 count ) {
	char crc=0;

	while ( count-- != 0 ) {
		*data = read_eeprom( address++ );
		crc = xor_crc(crc,*data);
		data++;
	}
	return crc;
}

char EEPROMDataWrite( int16 address, int8 *data, int16 count ) {
	char crc=0;

	while ( count-- != 0 ) {
		/* restart_wdt() */
		crc = xor_crc(crc,*data);
		write_eeprom( address++, *data++ );
	}

	return crc;
}

void write_param_file() {
	int8 crc;

	/* write the config structure */
	crc = EEPROMDataWrite(PARAM_ADDRESS,(void *)&config,sizeof(config));
	/* write the CRC was calculated on the structure */
	write_eeprom(PARAM_CRC_ADDRESS,crc);
}

void write_test_param_file() {
	/* red LED for 1.5 seconds */
	timers.led_on_green=150;

	config.modbus_address=24;
	config.modbus_speed=BAUD_9600; 
	config.modbus_mode=MODBUS_MODE_RTU;

	config.serial_prefix='A';
	config.serial_number=2702;

	config.adc_average_mode[0]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[1]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[2]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[3]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[4]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[5]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[6]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[7]=ADC_AVERAGE_NORMAL;
	config.adc_sample_ticks=20;

	config.worldData_seconds=60;
	config.worldData_trigger_prefix='A';
	config.worldData_trigger_number=2688;

	/* write them so next time we use from EEPROM */
	write_param_file();

}

void write_default_param_file() {
	/* red LED for 1.5 seconds */
	timers.led_on_green=150;

	config.modbus_address=24;
	//config.modbus_speed=BAUD_9600; 
	config.modbus_speed=BAUD_19200; 
	config.modbus_mode=MODBUS_MODE_RTU;

	config.serial_prefix='Z';
	config.serial_number=9876;

	config.adc_average_mode[0]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[1]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[2]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[3]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[4]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[5]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[6]=ADC_AVERAGE_NORMAL;
	config.adc_average_mode[7]=ADC_AVERAGE_NORMAL;
	config.adc_sample_ticks=20;

	config.worldData_seconds=60;
	config.worldData_trigger_prefix=0;
	config.worldData_trigger_number=0;

	config.io_enable=0;
	config.io_mode[IO_LINE_XBEE_SLEEP]=1;
	config.io_mode[IO_LINE_XBEE_RTS]=1;
	config.io_mode[IO_LINE_XBEE_CTS]=1;

	config.modbus_strobe_address=100; /* 256 => disabled */
	config.modbus_strobe_register=100;
	
	/* write them so next time we use from EEPROM */
	write_param_file();

}


void read_param_file() {
	int8 crc;

	crc = EEPROMDataRead(PARAM_ADDRESS, (void *)&config, sizeof(config)); 
		
	if ( crc != read_eeprom(PARAM_CRC_ADDRESS) ) {
		write_default_param_file();
	}
}


