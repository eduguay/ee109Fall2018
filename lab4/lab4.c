/********************************************
*
*  Name:		Eric Duguay
*  Email:		eduguay@usc.edu
*  Section:		W 2:00 pm
*  Assignment: Lab 4 - Up/Down counter on LCD display
*
********************************************/

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
		
		char count = 0; // Used it increment display
		char countUp = 1; // Used as a boolean to check if going up or down
		char i = 0; // Used in the loop to help with button checking
        
    while (1) {               // Loop forever

                // Use an outer if statement to select what state you are in
                // Then inside each 'if' or 'else' block, perform the desired
                // output operations and then sample the inputs to update
                // the state for the next iteration
        
                
                // Delay before we go to the next iteration of the loop
		
		//for loop to allow for the fast clicking of buttons
		for( i = 0; i < 10; i++){
			//changes countUP to 1 if the blue button is pressed
			if( ( PINC & (1 << 2) )  ==  0){
				countUp = 1;
				
			//changes countUp to 0 if the red button is pressed
			}else if( ( PINC & (1 << 4) )  ==  0){
				countUp = 0;
			}
			_delay_ms(50);
		}
		
		lcd_moveto(0,0);
		lcd_writedata(count + 0x30);
		// 
		
		
		if( countUp ){
			count++;
		} else {
			count--;
		}
		
		if( count >= 10 ) {
			count = 0;
		} else if ( count < 0 ) {
			count = 9;
		}
		
		
		
		
		
    }

    return 0;   /* never reached */
}

