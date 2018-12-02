/********************************************
 *
 *  Name:		Eric Duguay
 *  Email:		eduguay@usc.edu
 *  Section:	2:00 W
 *  Assignment: Project
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "ds18b20.h"

// Serial communications functions and variables
void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);
void clearScreen();

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register




volatile char buffer[5];		// Holds the messages
volatile int buffIndex = 0;		// Holds the index of the current
								// char incoming for buffer
volatile int finished = 0;		// If the array is finished processing
								// this flag is set to one
								
volatile char highOrLow = 1; 	// Flag for which temperature is currently
								// being changed by the rotary

volatile unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
volatile int count = 0;		// Count to display
volatile unsigned char a, b;

int main(void) {

    // Initialize the modules and LCD
    lcd_init();
    serial_init(MYUBRR);
	ds_init();
	
	
	// Set the pins to output for the two LEDS
	DDRD |= (1 << PD3);	// Green (Air conditioning ON) 
	DDRB |= (1 << PB5); // Red (Heating ON)
	
	// Setup PORT bits for the 2 input buttons and rotary
	PORTC |= (( 1 << 2 ) | ( 1 << 3 )); 		// Enable pull-up on A2, A3 buttons
	PORTB |= (( 1 << 3 ) |  (1 << 4)); 			// Enable pull-up on D11, D12 rotary
	PCMSK0 |= ((1 << PCINT3) | (1 << PCINT4)); 	//Enables interrupts for PB3 and PB4

    // Enable interrupts
	PCICR |= (1 << PCIE1) | (1 << PCIE0); 		// Enables pin change interrupts for PC and PB
	UCSR0B |= (1 << RXCIE0);    				// Enable receiver interrupts
    sei();                      				// Enable global interrupts

    // Show the splash screen for 1 second
	clearScreen();
	lcd_moveto(0,0);
	lcd_stringout("Eric Duguay");
	lcd_moveto(1,4);
	lcd_stringout("Project");
	_delay_ms(1000);
	lcd_moveto(0,0);
	clearScreen();
	
	/* For the Rotary: 
	*	Read the A and B inputs to determine the intial state  
	**/
	unsigned char cRegVal = PINC;		// Reads current value on that pin
	a = (cRegVal & (1 << 4));
	b = (cRegVal & (1 << 3));
	

	/* General Use Variables */
	unsigned char t[2];				// Used in order to house the output from the DS18B20	
	int currentTempDS = 0;			// Current temperature in the DS18B20 format
	unsigned char f = 0, lastF = 0; // Hold the current temp in farenheit and the last temp
	char outputString[20];			// A char array used to output to the LCD
	unsigned char high = eeprom_read_byte((void *) 200);	// Finds the previous high temp in memory
															// and stores it
	unsigned char low = eeprom_read_byte((void *) 300);		// Finds the previous low temp in memory
															// and stores it
	
	
	/* The initial screen output */
	snprintf(outputString, 20, "HIGH:%3d Low:%3d", high, low);
	lcd_moveto(0,0);
	lcd_stringout(outputString);
	lcd_moveto(1,0);
	lcd_stringout("Temp:    Rmt:");
	
	PORTD |= (1 << PD3);
	
    while (1) {                 // Loop forever
	
		/* Button Press Functionality */
		if( ( PINC & (1 << 2) )  ==  0){ 		// If button to adjust LOW is pressed
			highOrLow = 0;
			
			/* Indicate that LOW is now being changed */
			lcd_moveto(0,0);
			lcd_stringout("High");
			lcd_moveto(0,9);
			lcd_stringout("LOW");
		}else if( ( PINC & (1 << 3) )  ==  0){ 	// If button to adjust HIGH is pressed 
			highOrLow = 1;
			
			/* Indicate that HIGH is now being changed */
			lcd_moveto(0,0);
			lcd_stringout("HIGH");
			lcd_moveto(0,9);
			lcd_stringout("Low");
		}
		
		
		if(changed == 1){
			
			changed = 0;			// Returned changed to 0
			
			if(highOrLow == 1){		// If high is currently selected
				high += count;
				if(high > 100){		// Ensure high does not go above 100
					high = 100;
				}
				if(high < low){		// Ensure high does not go below low
					high = low;
				}
				/* Save the new high to memory */
				eeprom_update_byte((void *) 200, high); 
			}else{					// If low is currently selected
				low += count;
				if(low < 40){		// Ensure low does not go below 40
					low = 40;
				}
				if(low > high){		// Ensure low does not go above high
					low = high;
				}
				/* Save the new low to memory*/
				eeprom_update_byte((void *) 300, low);
			}
			
			count = 0;				// Return the rotary count to 0
			
			/* Update the screen with the new high and low values*/
			lcd_moveto(0,5);
			snprintf(buffer, 4, "%3d", high);
			lcd_stringout(buffer);
			lcd_moveto(0,13);
			snprintf(buffer, 4, "%3d", low);
			lcd_stringout(buffer);
		}
		
		/* Logic to measure and convert the temperature */
		ds_temp(t);									// Read temperature
		currentTempDS = ( t[1] & 0x07 );			// Copy first 3 bits from DS18B20
		currentTempDS <<= 8;						// Shift bits by 8
		currentTempDS |= t[0];						// Copy next 8 bits from DS18B20
		f = ( 9 * currentTempDS ) / (5 * 16) + 32;	// Convert DS18B20 input to farenheit
		
		/* Print out the current temperature on lcd */
		lcd_moveto(1,5);
		snprintf(outputString, 10, "%3d", f);
		lcd_stringout(outputString);
		
		/* If the temperature has changed output on serial connection */
		if(lastF != f){
			lastF = f; 					// reset the last temperature
			char tsend[6];				// Declare a char array with the data to send
			tsend[0] = '@';				// Set the first character in the array to '@'
			
			/* If the temperature is above zero add a '+'*/
			if(f > 0){
				tsend[1] = '+';
			}
			
			/* Add the measured temperature to the outputString as a holding */ 
			snprintf(outputString, 4, "%d", f);
			
			/* Logic to add specific temperatures to the output*/
			if(f >= 100){ 						// Three digit positives
				tsend[2] = outputString[0];
				tsend[3] = outputString[1];
				tsend[4] = outputString[2];
				tsend[5] = '$';
			} else if( f < -9){					// Two digit negatives
				tsend[2] = outputString[0];
				tsend[3] = outputString[1];
				tsend[4] = outputString[2];
				tsend[5] = '$';
			} else if ( f > 9){					// Two digit positives
				tsend[2] = outputString[0];
				tsend[3] = outputString[1];
				tsend[4] = '$';
			} else if ( f < 0){					// One digit negatives
				tsend[2] = outputString[0];
				tsend[3] = outputString[1];
				tsend[4] = '$';
			} else {							// One digit positives
				tsend[2] = outputString[0];
				tsend[3] = '$';
			}
			serial_stringout(tsend);			// Sends the string out serial
			
			/* Improves performance - unknown reason */
			lcd_moveto(0,16);
			lcd_stringout("tsend");
		}
		
		
		/* Activates when the arduino is finished recieving output */
		if(finished == 1){
			finished = 0;				// Reset the finished variables
			buffIndex = 0;				// Reset the buffer Index
			
			/* Clear the part of the screen with external temp */
			lcd_moveto(1,13);
			lcd_stringout("   ");
			lcd_moveto(1,13);
			
			/* Convert the buffer to an integer and then that integer
			*	back to a string. This ensures the output is properly formed.
			* 	Finally, output the string.
			*/
			int oTemp;
			sscanf(buffer, "%d", &oTemp);
			snprintf(outputString, 10, "%3d", oTemp);
			lcd_stringout(outputString);
		}
		
		/* Turns the air conditioning light on if the temp is too high*/
		if( high < f ){
			PORTD |= (1 << PD3);
		} else {
			PORTD &= ~(1 << PD3);
		}
	
		/* Turns the heater light on if the temp is too low */
		if(low > f){
			PORTB |= (1 << PB5);
		} else {
			PORTB &= ~(1 << PB5);
		}
    }
}

/* ----------------------------------------------------------------------- */

void serial_init(unsigned short ubrr_value)
{

    // Set up USART0 registers
	UBRR0 = ubrr_value;             	  // Set baud rate
	UCSR0B |= (1 << TXEN0 | 1 << RXEN0);  // Enable RX and TX
	UCSR0C = (3 << UCSZ00);               // Async., no parity,
                                          // 1 stop bit, 8 data bits
	// Enable tri-state
	DDRD |= (1 << PD2);					  // Enables PD2 as output
	PORTD &= ~( 1 << PD2 );				  // Sets PD2 to 0
	
}

/* Sends a char out */
void serial_txchar(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

/* Sends a string out */
void serial_stringout(char *s)
{
	
	// Call serial_txchar in loop to send a string
	int i = 0;
	/* Loop through the input string and break if reaches the end */
	for(i = 0; i < 6; i++){
		serial_txchar(s[i]);
		if(s[i] == '$') break;
	}

}

/* Interrupt when recieving a serial input*/
ISR(USART_RX_vect)
{

    // Handle received character
	char ch;
	
	ch = UDR0;		// Get the received charater

    // If message complete, set flag
	if(ch == '@'){
		// DO NOTHING
	}else if(ch == '$'){
		// If the end of the string add a terminating character and set
		// finished flag to true
		buffer[buffIndex++] = '\0';
		finished = 1;
	} else {
		// Add the character to the buffer string
		buffer[buffIndex++] = ch;
	}
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

/* Rotary interrupt */
ISR(PCINT0_vect){
	
	// Read the input bits and determine A and B
	unsigned char cRegVal = PINB;
	a = (cRegVal & (1 << 4));
	b = (cRegVal & (1 << 3));
	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the count value.
	if (old_state == 0) {
	    // Handle A and B inputs for state 0
		if(a&&!b){
			new_state = 1;
			count++;
		}
		if(!a&&b){
			new_state = 3;
			count--;
		}
	}
	else if (old_state == 1) {
	    // Handle A and B inputs for state 1
		if(a&&b){
			new_state = 2;
			count++;
		}
		if(!a&&!b){
			new_state = 0;
			count--;
		}
	}
	else if (old_state == 2) {
	    // Handle A and B inputs for state 2
		if(!a&&b){
			new_state = 3;
			count++;
		}
		if(a&&!b){
			new_state = 1;
			count--;
		}
	}
	else {   // old_state = 3
	    // Handle A and B inputs for state 3
		if(!a&&!b){
			new_state = 0;
			count++;
		}
		if(a&&b){
			new_state = 2;
			count--;
		}	
	}


	// If state changed, update the value of old_state,
	// and set a flag that the state has changed.
	if (new_state != old_state) {
	    changed = 1;
	    old_state = new_state;
	}
}
