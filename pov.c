// (c) 2016-02-07 Jens Hauke <jens.hauke@4k2.de>
//
// ATMEL ATTINY13a
//
//       +-\/-+
// PB5  1|    |8  Vcc
// PB3  2|    |7  PB2
// PB4  3|    |6  PB1 PWM
// GND  4|    |5  PB0 PWM
//       +----+
//
// LEDs on PB0-PB4
// A very simple persistence of vision (POV) display with five LEDs for ATTiny13.

#include <inttypes.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#if !defined(BODSE) && defined(BPDSE)
/* fix typo in older versions of avr/iotn13a.h (included by avr/io.h) */
#define BODSE BPDSE
#define BODS BPDS
#endif
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>


#define PIXEL_PER_LOOP	120
#define LOOPS_PER_SECOND 6
#define PIXEL_DELAY	(1000000 /*µs*/ / ((LOOPS_PER_SECOND) * (PIXEL_PER_LOOP)))
#define SLEEP_AFTER	(60 /*seconds*/)


static inline
void init() {
	DDRB  = 0b00011111; // PB0-4 output
	PORTB = 0b00000000; // all low
}


void power_down(void) {
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Port state
	DDRB  = 0b11111111;		// PB all output
	PORTB = 0b00000000;		// PB all low

	// ADC
	uint8_t adcsra = ADCSRA;	//save ADCSRA
	ADCSRA &= ~_BV(ADEN);		//disable ADC. Important to safe power.

	// Sleep with BOD disabled (see avr/sleep.h)
	// ~4.5µA @3.3V with WDT enabled
	// ~6.2µA @5.0V with WDT enabled
	cli();
	if (1) {
		sleep_enable();
		sleep_bod_disable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
	sei();

	ADCSRA = adcsra;		//restore ADCSRA
}


const uint8_t data[] PROGMEM =
#if 1
	// Generated pattern
#	include "banner1.inc"
#else
	// Test pattern
{
	0b00011111, // 0x1f
	0b00001111, // 0x04
	0b00000111, // 0x04
	0b00000011, // 0x1f
	0b00000001, // 0x00
	0b00000001, // 0x1f
	0b00000001, // 0x15
	0b00000001, // 0x15
	0b00000010, // 0x15
	0b00000100, // 0x00
	0b00001000, // 0x1f
	0b00010000, // 0x10
	0b00010000, // 0x10
	0b00010000, // 0x10
	0b00001000, // 0x00
	0b00001000, // 0x1f
	0b00001000, // 0x10
	0b00000100, // 0x10
	0b00000100, // 0x10
	0b00000100, // 0x00
	0b00000010, // 0x0e
	0b00000010, // 0x11
	0b00000010, // 0x11
	0b00000001, // 0x0e
	0b00000001, // 0x00
	0b00000001, // 0x00
	0b00000001, // 0x00
	0b00000001, // 0x0f
	0b00000000, // 0x10
	0b00000001, // 0x0c
	0b00000000, // 0x10
	0b00011111, // 0x0f
}
#endif
	;

static
void sequence(void) {
	uint8_t i;

	for (i = 0; i < sizeof(data); i++) {
		uint8_t val = pgm_read_byte(data + i);
		PORTB = val;
		_delay_us(PIXEL_DELAY);
	}
	PORTB = 0; // All off.
	_delay_us((PIXEL_PER_LOOP - sizeof(data)) * PIXEL_DELAY);
}


static
void loop() {
	unsigned i;

	// Show sequence for SLEEP_AFTER seconds
	for (i = 0; i < (SLEEP_AFTER * LOOPS_PER_SECOND); i++) {
		sequence();
	}

	power_down(); // Wake up by reset.
}


int main(int argc, char **argv)
{
	init();
	while (1) loop();

	return 0;
}
