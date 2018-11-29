/********************************************
 *
 *  Name:		Eric Duguay
 *  Email:		eduguay@usc.edu
 *  Section:	2:00 pm
 *  Assignment: Lab 6 - Analog-to-digital conversion
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "adc.h"



void clearScreen()
{
	lcd_moveto(0,0);
	lcd_stringout("                  ");
	lcd_moveto(1,0);
	lcd_stringout("                  ");
	lcd_moveto(0,0);

	
}


int main(void)
{
    unsigned char adc_result;


    /* Initialize the LCD */
	lcd_init();
	lcd_moveto(0,0);

    // Initialize the ADC
	adc_init( 0x03 );

    // Use the ADC to find a seed for rand();
    adc_result = adc_sample();
    srand(adc_result << 8 | adc_result);  // Make a 16-bit number for srand()

    // Write splash screen
	/*Clear screen*/
	clearScreen();
	
	lcd_stringout("Eric Duguay");
	lcd_moveto(1,4);
	lcd_stringout("Lab 6");
	_delay_ms(1000);
	
    // Find a random initial position of the 'X'
	int startPosition = rand() % 16;

	clearScreen();

    // Display the 'X' on the screen
	lcd_moveto(0, startPosition);
	lcd_stringout("X");


	char adc_last = 0;
	int count = 0;
	
    while (1) {                 // Loop forever
        // Do a conversion
	adc_result = adc_sample();
	adc_result = adc_result / 16;

	// Move '^' to new position
	
	//Only updates the screen if the position has changed
	if(adc_last != adc_result){
		lcd_moveto(1, adc_last);
		lcd_stringout(" ");\
		lcd_moveto(1,adc_result);
		lcd_stringout("^");
	}
	
	
	//Used to erase the last position and to check for movement
	adc_last = adc_result;
	
	// Delay
	
	_delay_ms(10); 

	// Check if '^' is aligned with 'X'

	if(adc_result == startPosition){
		count++;
	} else {
		count = 0;
	}
	
	//If it has been aligned for 2 seconds print out "You Win!"
	if(count == 200){
		clearScreen();
		lcd_moveto(0,0);
		lcd_stringout("You Win!         ");
		while(1){
			
		}
	}
	
    }

    return 0;   /* never reached */
}

