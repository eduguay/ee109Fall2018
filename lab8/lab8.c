/********************************************
 *
 *  Name:		Eric Duguay
 *  Email:		eduguay@usc.edu
 *  Section:	2:00 pm
 *  Assignment: Lab 8 - Digital Stopwatch
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"




/* Determines whether or not the LCD needs to be updated */
volatile unsigned char update = 0;
/* Holds the state of the stopwatch */
volatile unsigned char state = 0;
int main(void)
{
    unsigned char adc_result;


    /* Initialize the LCD */
	lcd_init();
	lcd_moveto(0,0);

    // Initialize the ADC on channel zero
	adc_init( 0x00 );

	
	/* Set the mode for "Clear Timer on Compare" */
	TCCR1B |= (1 << WGM12);
	
	
	/* Enaable "Output Compare A Match Interrupt" */
	TIMSK1 |= (1 << OCIE1A);
	
	/*Load the 16-bit counter modulus into OCR1A */
	OCR1A = 25000;
	
	/* Set prescalar = 64 and start counter */
	TCCR1B |= (1 << CS11) | (1 << CS10);
	
    // Write splash screen
	clearScreen();
	lcd_stringout("Eric Duguay");
	lcd_moveto(1,4);
	lcd_stringout("Lab 8");
	_delay_ms(1000);
	clearScreen();
	
	/*The char string to hold the time to print as well 
	as the variables which hold the individual charater
	values. */
	char time[10];
	char tenths = 0;
	char ones = 0;
	char tens = 0;
	
	/* Enables interrupts */
	sei();
	
	/* Prints the initial time of "00.0"*/
	lcd_moveto(1,0);
	snprintf(time, 5, "%d%d.%d", tens,ones,tenths);
	lcd_stringout(time);
	
    while (1) {                 // Loop forever
	
	/* Polls the ADC for button presses */
	adc_result = adc_sample();
	
	/* Checks if the UP button has been pushed */
	if((adc_result < 60) && (adc_result > 40)){
		// If stopped go to started
		if(state == 0){
			state = 1;
		// If started go to stopped
		}else if(state == 1){
			state = 0;
		// If lapped go to started
		}else if(state == 2){
			state = 1;
		}
		_delay_ms(150);
	/* Checks if the UP button has been pushed */
	}else if((adc_result < 110) && (adc_result > 90)){
		// If stopped reset and print
		if(state == 0){
			tenths = 0;
			ones = 0;
			tens = 0;
			lcd_moveto(1,0);
			snprintf(time, 5, "%d%d.%d", tens,ones,tenths);
			lcd_stringout(time);
		// If started go to lapped
		}else if(state == 1){
			state = 2;
		// If lapped go to started
		}else if(state == 2){
			state = 1;
		}
		_delay_ms(150);
	}
	
	/* Updates the state of the stopwatch as needed */
	if(state == 0){
		
	}

	/* Checks a update to see if an update of the displayed time
	on the LCD is required */
	if(update && state){
		update = 0; 
		
		tenths++;
		//Overflow to ones place
		if(tenths == 10){
			tenths = 0;
			ones++;
		}
		
		//Overflow to tens place
		if(ones == 10){
			ones = 0;
			tens++;
		}
		
		//Completely reset if reach a minute
		if(tens == 6){
			tens = 0;
		}

		// Will outprint the result only if it is not in the Lapped state
		if(state != 2){
			lcd_moveto(1,0);
			snprintf(time, 5, "%d%d.%d", tens,ones,tenths);
			lcd_stringout(time);
		}
	}
	}

    return 0;   /* never reached */
}

/* Interrupt which updates the update variable every .1 second */
ISR(TIMER1_COMPA_vect){
	update = 1;
}

/*Clears the screen*/
void clearScreen()
{
	lcd_moveto(0,0);
	lcd_stringout("                  ");
	lcd_moveto(1,0);
	lcd_stringout("                  ");
	lcd_moveto(0,0);

	
}