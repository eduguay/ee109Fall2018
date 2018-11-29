/********************************************
*
*  Name:		Eric Duguay
*  Email:		eduguay@usc.edu
*  Section:		W 2:00 pm
*  Assignment: Lab 4 - Up/Down counter on LCD display
*
********************************************/

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

int main(void) {

	// Setup DDR and PORT bits for the 2 input buttons
	// as necessary
	
	PORTC |= ( 1 << 2 );
	PORTC |= ( 1 << 4 );
	
	
	// Initialize the LCD
	lcd_init();
	lcd_moveto(0,0);
	
	// Use a state machine approach to organize your code
	//   - Declare and initialize a variable to 
	//     track what state you are in
	
	/* Call lcd_stringout to print out your name */
	lcd_stringout("Eric Duguay");
	/* Use snprintf to create a string with your birthdate */
	char date[30];
	snprintf(date, 30, "%d/%d/%d", 1, 11, 1998);
	/* Use lcd_moveto to start at an appropriate column 
		in the bottom row to appear centered */
	lcd_moveto();
	/* Use lcd_stringout to print the birthdate string */

	/* Delay 1 second */
	
        
    while (1) {               // Loop forever

               
    }

    return 0;   /* never reached */
}

