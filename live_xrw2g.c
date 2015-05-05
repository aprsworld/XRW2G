int16 crc_chk(int8 *data, int8 length) {
	int8 j;
	int16 reg_crc=0xFFFF;

	while ( length-- ) {
		reg_crc ^= *data++;

		for ( j=0 ; j<8 ; j++ ) {
			if ( reg_crc & 0x01 ) {
				reg_crc=(reg_crc>>1) ^ 0xA001;
			} else {
				reg_crc=reg_crc>>1;
			}
		}	
	}
	
	return reg_crc;
}


/*		
'#'                   0  STX
UNIT ID PREFIX        1  First character (A-Z) for serial number
UNIT ID MSB           2  high byte of sending station ID
UNIT ID LSB           3  low byte of sending station ID
PACKET LENGTH         4  number of byte for packet including STX through CRC
PACKET TYPE           5  type of packet we are sending, 23
SEQUENCE MSB          6
SEQUENCE LSB          7

PULSE_COUNT[0] MSB    8  
PULSE_COUNT[0] LSB    9
PULSE_TIME[0] MSB     10
PULSE_TIME[0] LSB     11
PULSE_MIN_TIME[0] MSB 12
PULSE_MIN_TIME[0] LSB 13
PULSE_MAX_TIME[0] MSB 14
PULSE_MAX_TIME[0] LSB 15
PULSE_SUM[0] MSB      16
PULSE_SUM[0]          17
PULSE_SUM[0]          18
PULSE_SUM[0] LSB      19

PULSE_COUNT[1] MSB    20  
PULSE_COUNT[1] LSB    21
PULSE_TIME[1] MSB     22
PULSE_TIME[1] LSB     23
PULSE_MIN_TIME[1] MSB 24
PULSE_MIN_TIME[1] LSB 25
PULSE_MAX_TIME[1] MSB 26
PULSE_MAX_TIME[1] LSB 27
PULSE_SUM[1] MSB      28
PULSE_SUM[1]          29
PULSE_SUM[1]          30
PULSE_SUM[1] LSB      31

PULSE_COUNT[2] MSB    32 
PULSE_COUNT[2] LSB    33
PULSE_TIME[2] MSB     34
PULSE_TIME[2] LSB     35
PULSE_MIN_TIME[2] MSB 36
PULSE_MIN_TIME[2] LSB 37
PULSE_MAX_TIME[2] MSB 38
PULSE_MAX_TIME[2] MSB  39
PULSE_SUM[2] MSB      40
PULSE_SUM[2]          41
PULSE_SUM[2]          42
PULSE_SUM[2] LSB      43

ANALOG_CURRENT[0] MSB 44
ANALOG_CURRENT[0] LSB 45
ANALOG_AVG[0] MSB     46
ANALOG_AVG[0] LSB     47
ANALOG_STD_DEV[0] MSB 48
ANALOG_STD_DEV[0] LSB 49

ANALOG_CURRENT[1] MSB 50
ANALOG_CURRENT[1] LSB 51
ANALOG_AVG[1] MSB     52
ANALOG_AVG[1] LSB     53
ANALOG_STD_DEV[1] MSB 54
ANALOG_STD_DEV[1] LSB 55

ANALOG_CURRENT[2] MSB 56
ANALOG_CURRENT[2] LSB 57
ANALOG_AVG[2] MSB     58
ANALOG_AVG[2] LSB     59
ANALOG_STD_DEV[2] MSB 60
ANALOG_STD_DEV[2] LSB 61

ANALOG_CURRENT[3] MSB 62
ANALOG_CURRENT[3] LSB 63
ANALOG_AVG[3] MSB     64
ANALOG_AVG[3] LSB     65
ANALOG_STD_DEV[3] MSB 66
ANALOG_STD_DEV[3] LSB 67

ANALOG_CURRENT[4] MSB 68
ANALOG_CURRENT[4] LSB 69
ANALOG_AVG[4] MSB     70
ANALOG_AVG[4] LSB     71
ANALOG_STD_DEV[4] MSB 72
ANALOG_STD_DEV[4] LSB 73

ANALOG_CURRENT[5] MSB 74
ANALOG_CURRENT[5] LSB 75
ANALOG_AVG[5] MSB     76
ANALOG_AVG[5] LSB     77
ANALOG_STD_DEV[5] MSB 78
ANALOG_STD_DEV[5] LSB 79

ANALOG_CURRENT[6] MSB 80
ANALOG_CURRENT[6] LSB 81
ANALOG_AVG[6] MSB     82
ANALOG_AVG[6] LSB     83
ANALOG_STD_DEV[6] MSB 84
ANALOG_STD_DEV[6] LSB 85

ANALOG_CURRENT[7] MSB 86
ANALOG_CURRENT[7] LSB 87
ANALOG_AVG[7] MSB     88
ANALOG_AVG[7] LSB     89
ANALOG_STD_DEV[7] MSB 90
ANALOG_STD_DEV[7] LSB 91

UPTIME_MINUTES MSB    92
UPTIME_MINUTES LSB    93

INTERVAL MS MSB       94
INTERVAL MS LSB       95

CRC MSB               96 high byte of CRC on everything after STX and before CRC
CRC LSB               97 low byte of CRC
*/


void live_send(void) {
	int8 buff[98];
	int16 lCRC;
	int8 i;
	int8 base;
	int16 l;
	int32 ll;

	timers.led_on_green=50;

	buff[0]='#';
	buff[1]=config.serial_prefix;
	buff[2]=make8((int16) config.serial_number,1);
	buff[3]=make8((int16) config.serial_number,0);
	buff[4]=sizeof(buff); /* packet length */
	buff[5]=23; /* packet type */
	buff[6]=make8(current.sequence_number,1);
	buff[7]=make8(current.sequence_number,0);

	/* counter channels */
	for ( i=0 ; i<3 ; i++ ) {
		base=i*12+8;
		buff[base+0 ]=make8(current.pulse_count[i],1);
		buff[base+1 ]=make8(current.pulse_count[i],0);
		buff[base+2 ]=make8(current.pulse_period[i],1);
		buff[base+3 ]=make8(current.pulse_period[i],0);
		buff[base+4 ]=make8(current.pulse_min_period[i],1);
		buff[base+5 ]=make8(current.pulse_min_period[i],0);
		buff[base+6 ]=make8(current.pulse_max_period[i],1);
		buff[base+7 ]=make8(current.pulse_max_period[i],0);

		ll=get_pulse_sum(i);
		buff[base+8 ]=make8(ll,3);	
		buff[base+9 ]=make8(ll,2);
		buff[base+10]=make8(ll,1);
		buff[base+11]=make8(ll,0);
	}

	/* analog channels */
	for ( i=0 ; i<8 ; i++ ) {
		base=i*6+44;
		
		l=current.adc_buffer[i][current.adc_buffer_index];
		buff[base+0]=make8(l,1);
		buff[base+1]=make8(l,0);

		l=adc_get(i);
		buff[base+2]=make8(l,1);
		buff[base+3]=make8(l,0);

		buff[base+4]=0;
		buff[base+5]=0;
	}

	buff[92]=make8(current.uptime_minutes,1);
	buff[93]=make8(current.uptime_minutes,0);

	disable_interrupts(GLOBAL);
	buff[94]=make8(current.interval_10milliseconds,1);
	buff[95]=make8(current.interval_10milliseconds,0);
	enable_interrupts(GLOBAL);

	lCRC=crc_chk(buff+1,95);
	buff[96]=make8(lCRC,1);
	buff[97]=make8(lCRC,0);

	output_low(XBEE_SLEEP);
	if ( 0 == config.worldData_trigger_prefix ) 
		delay_ms(14);

	for ( i=0 ; i<sizeof(buff) ; i++ ) {
		/* xbee modem */
		fputc(buff[i],MODBUS_SERIAL);
	}	


	/* only go to sleep if we aren't supposed to listen for anything else */
	if ( 0 == config.worldData_trigger_prefix ) 
		output_high(XBEE_SLEEP);

	current.sequence_number++;
	reset_counters();


	current.interval_10milliseconds=0;
}

