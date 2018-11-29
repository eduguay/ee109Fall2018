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
#include "adc.h"
#include "ds18b20.h"

// Serial communications functions and variables
void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);
void clearScreen();

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register

// ADC functions and variables
void adc_init(unsigned char);

#define ADC_CHAN 0              // Buttons use ADC channel 0


volatile char buffer[5];		// Holds the messages
volatile int buffIndex = 0;		// Holds the index of the current
								// char incoming for buffer
volatile int finished = 0;		// If the array is finished processing
								// this flag is set to one
								
volatile char hTempChanged = 1, lTempChanged = 0;


volatile unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
volatile int count = 0;		// Count to display
volatile unsigned char a, b;

int main(void) {

    // Initialize the modules and LCD
    lcd_init();
    adc_init(ADC_CHAN);
    serial_init(MYUBRR);
	ds_init();
	
	// Setup DDR and PORT bits for the 2 input buttons
	// as necessary
	
	DDRD |= (1 << PD3);
	DDRB |= (1 << PB5);
	
	PORTC |= (( 1 << 2 ) | ( 1 << 3 )); // Enable pull-up on A2, A3 buttons
	PCMSK1 |= ((1 << PCINT10) | (1 << PCINT11)); //Enables interrupts for PC2 and PC3
	PORTB |= (( 1 << 3 ) |  (1 << 4)); 	// Enable pull-up on D11, D12 rotary
	PCMSK0 |= ((1 << PCINT3) | (1 << PCINT4)); //Enables interrupts for PB3 and PB4

    // Enable interrupts
	PCICR |= (1 << PCIE1) | (1 << PCIE0); // Enables pin change interrupts for PC and PB
	UCSR0B |= (1 << RXCIE0);    // Enable receiver interrupts

    sei();                      // Enable interrupts

    // Show the splash screen
	clearScreen();
	lcd_moveto(0,0);
	lcd_stringout("Eric Duguay");
	lcd_moveto(1,4);
	lcd_stringout("Project");
	_delay_ms(1000);
	lcd_moveto(0,0);
	clearScreen();
	
	// Read the A and B inputs to determine the intial state
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.    

	unsigned char cRegVal = PINC;
	
	a = (cRegVal & (1 << 4));
	b = (cRegVal & (1 << 3));
								
	unsigned char t[2];						
	int a = 0;	
	unsigned char f = 0, lastF = 0;
	char tempOutput[20], highLowOutput[20];
	unsigned char high = eeprom_read_byte((void *) 200), low = eeprom_read_byte((void *) 300);
	
	
	snprintf(highLowOutput, 20, "HIGH:%3d Low:%3d", high, low);

	
	lcd_moveto(0,0);
	
	
	lcd_stringout(highLowOutput);
	lcd_moveto(1,0);
	lcd_stringout("Temp:    Rmt:");
	
	PORTD |= (1 << PD3);
	
    while (1) {                 // Loop forever
	
		
		\
			if( ( PINC & (1 << 2) )  ==  0){
				lTempChanged = 1;
				hTempChanged = 0;
				lcd_moveto(0,0);
				lcd_stringout("High");
				lcd_moveto(0,9);
				lcd_stringout("LOW");
			}else if( ( PINC & (1 << 3) )  ==  0){
				hTempChanged = 1;
				lTempChanged = 0;
				lcd_moveto(0,0);
				lcd_stringout("HIGH");
				lcd_moveto(0,9);
				lcd_stringout("Low");
			}
			_delay_ms(5);
		
		
		if(changed == 1){
			changed = 0;
			if(hTempChanged == 1){
				high += count;
				if(high > 100){
					high = 100;
				}
				if(high < low){
					high = low;
				}
				eeprom_update_byte((void *) 200, high);
			}else{
				low += count;
				if(low < 40){
					low = 40;
				}
				if(low > high){
					low = high;
				}
				eeprom_update_byte((void *) 300, low);
			}
			count = 0;
			lcd_moveto(0,5);
			snprintf(buffer, 4, "%3d", high);
			lcd_stringout(buffer);
			lcd_moveto(0,13);
			snprintf(buffer, 4, "%3d", low);
			lcd_stringout(buffer);
			
			lcd_stringout(highLowOutput);
		}
		

		ds_temp(t);
		a = ( t[1] & 0x07 );
		a <<= 8;
		a |= t[0];
		f = ( 9 * a ) / (5 * 16) + 32;
		lcd_moveto(1,5);
		snprintf(tempOutput, 10, "%3d", f);
		lcd_stringout(tempOutput);
		
		
		if(lastF != f){
			lastF = f;
			char tsend[6];
			tsend[0] = '@';
			if(f < 0){
				tsend[1] = '-';
			} else {
				tsend[1] = '+';
			}
			snprintf(highLowOutput, 4, "%d", f);
			if(f > 100){
				tsend[2] = highLowOutput[0];
				tsend[3] = highLowOutput[1];
				tsend[4] = highLowOutput[2];
				tsend[5] = '$';
			} else if( f < -9){
				tsend[2] = highLowOutput[0];
				tsend[3] = highLowOutput[1];
				tsend[4] = '$';
			} else if ( f > 9){
				tsend[2] = highLowOutput[0];
				tsend[3] = highLowOutput[1];
				tsend[4] = '$';
			} else{
				tsend[2] = highLowOutput[0];
				tsend[3] = '$';
			}
			serial_stringout(tsend);
			lcd_moveto(0,16);
			lcd_stringout("tsend");
		}
		
		
		if(finished == 1){
			finished = 0;
			buffIndex = 0;
			lcd_moveto(1,13);
			lcd_stringout("   ");
			lcd_moveto(1,13);
			int oTemp;
			sscanf(buffer, "%d", &oTemp);
			snprintf(highLowOutput, 10, "%3d", oTemp);
			//snprintf(highLowOutput, 10, "%4d", buffer);
			/*
			char toPrint[3];
			toPrint[0] = buffer[1];
			toPrint[1] = buffer[2];
			toPrint[2] = buffer[3];
			_delay_ms(1);
			*/
			lcd_stringout(highLowOutput);
		}
		
		
		if( high < f ){
			PORTD |= (1 << PD3);
		} else {
			PORTD &= ~(1 << PD3);
		}
	
	
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
	for(i = 0; i < 6; i++){
		
		serial_txchar(s[i]);
		if(s[i] == '$') break;
	}
	/*
	while(s[i] != '$'){
		
		serial_txchar(s[i]);
		//lcd_moveto(1,14);
		//lcd_stringout(s[i]);
		i = i + 1;
	}
    */

}

/* Interrupt when recieving a serial input*/
ISR(USART_RX_vect)
{

    // Handle received character
	char ch;
	
	ch = UDR0;		// Get the received charater
	
	
	// Store in buffer and increment bufferIndex
	

    // If message complete, set flag
	if(ch == '@'){
		
	}else if(ch == '$'){
		buffer[buffIndex++] = '\0';
		finished = 1;
	} else {
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

ISR(PCINT1_vect){
	/*
	unsigned char checkButton = PINC;
	if(checkButton & ( 1 << 2 ) == 1){
		hTempChanged = 1;
		lTempChanged = 0;
		lcd_moveto(0,0);
		lcd_stringout("HIGH");
	}else if(checkButton & ( 1 << 3 ) == 0){
		hTempChanged = 0;
		lTempChanged = 1;
	}
	*/
}
