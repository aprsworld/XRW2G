#include <18F4523.h>
#device ADC=12 
#device *=16

/* leave pages 0 and 1 alone for bootloader */
//#build(reset=0x800,interrupt=0x808)
//#org 0x0000,0x07ff {}

#include <stdlib.h>
#FUSES INTRC_IO,NOPROTECT,PUT,NOLVP,BROWNOUT,NOMCLR,WDT32768
#use delay(clock=8000000)

#use standard_io(A)
#use standard_io(B)
#use standard_io(C)
#use standard_io(D)
#use standard_io(E)



/* IO */
#define LED_GREEN   PIN_C2
#define LED_RED     PIN_C3

#define SYNC_IN     PIN_B4
#define GPS_PPS     PIN_B5

#define XBEE_CTS    PIN_D3
#define XBEE_RTS    PIN_D2
#define XBEE_SLEEP  PIN_D1

#define RS485_DE    PIN_C4
#define RS485_NRE   PIN_C5

#define ADC_MUX_SEL PIN_C0

#define TP1         PIN_D4
#define TP2         PIN_D5

#define JP_WORLDDATA PIN_B6

/* analog channels */
#define AN_USER_USER_0 4
#define AN_USER_USER_1 5
#define AN_USER_USER_2 7
#define AN_USER_USER_3 6

#define AN_TEMPERATURE 9
#define AN_IN_VOLTS    0
#define AN_WIND_DIR_0  1
#define AN_WIND_DIR_1  2


/* EEPROM locations */
#define PARAM_CRC_ADDRESS 0x00
#define PARAM_ADDRESS     0x02


#define ADC_AVERAGE_NORMAL 0
#define ADC_AVERAGE_VECTOR 1


/* Modbus defines */
#define BAUD_9600  0
#define BAUD_19200 1

#define MODBUS_MODE_RTU     0
#define MODBUS_MODE_TCP_RTU 1

#define IO_LINE_XBEE_SLEEP  0
#define IO_LINE_XBEE_RTS    1
#define IO_LINE_XBEE_CTS    2

typedef union {
	int16 l[2];
    int8 b[4];
    int32 word;
} u_lblock;

#byte PORTB=GETENV("SFR:portb")