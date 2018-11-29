#include <avr/io.h>

#include "adc.h"


void adc_init(unsigned char channel)
{
    // Initialize the ADC
	
	/* Step 1: Choose AVCC and give range of 0-5V */
	ADMUX |= ( 1 << 6 );
	
	/* Step 2: Set MUX bits in ADMUX register to desired channel number */
	ADMUX |= channel;
	
	/* Step 3: Set prescalar to 128 */
	ADCSRA |= 0x07;
	
	/* Step 4: Set ADLAR bit to 1 in ADMUX */
	ADMUX |= ( 1 << 5 );
	
	/* Step 5: Set ADEN bit to 1 */
	ADCSRA |= ( 1 << 7);
}


unsigned char adc_sample()
{
    // Convert an analog input and return the 8-bit result

	/* Step 1: Set the ADSC bit in the ADCSRA register */
	ADCSRA |= ( 1 << ADSC );
	
	/* Step 2: Check whether ADSC bit has turned back to 0 */
	while((ADCSRA & ( 1 << ADSC ) ) != 0) {}
	
	/* Step 3: Read the 8-bit value from ADCH register */
	unsigned char result = ADCH;
	
	return result;
}
