/********************************************
 *
 *  Name:		Eric Duguay
 *  Email:		eduguay@usc.edu
 *  Section:	2:00 W
 *  Assignment: Lab 9 - Serial Communications
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"

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

/* Holds the different messages which can be sent */
char *messages[] = {
    "Hello           ",
    "How are you?    ",
	"Good. You?      ",
	"Bad. You?       ",
	"Sad. You?       ",
	"Glad. You?      ",
	"Mad. You?       ",
	"Chad. You?      ",
	"Fad. You?       ",
	"Fantastic.      "
};


volatile char buffer[16];		// Holds the messages
volatile int buffIndex = 0;		// Holds the index of the current
								// char incoming for buffer
volatile int finished = 0;		// If the array is finished processing
								// this flag is set to one
int main(void) {

    // Initialize the modules and LCD
    lcd_init();
    adc_init(ADC_CHAN);
    serial_init(MYUBRR);

    // Enable interrupts
	UCSR0B |= (1 << RXCIE0);    // Enable receiver interrupts

    sei();                      // Enable interrupts

    // Show the splash screen
	clearScreen();
	lcd_moveto(0,0);
	lcd_stringout("Eric Duguay");
	lcd_moveto(1,4);
	lcd_stringout("Lab 9");
	_delay_ms(1000);
	lcd_moveto(0,0);
	clearScreen();
	
	
	unsigned char adc_result;	// Holds the value of adc input
	int messageIndex = 0;		// Holds the index of the message
								// currently displayed on lcd
	//Prints out the default message at index 0
	lcd_stringout(messages[0]);	
    while (1) {                 // Loop forever
		
        // Get an ADC sample
		adc_result = adc_sample();

        // Up button pressed
		if((adc_result < 60) && (adc_result > 40)){
			lcd_moveto(0,0);
			
			//Cycles message to make sure it doesn't go out of bounds
			if(messageIndex == 0){
				messageIndex = 9;
			}else{
				messageIndex--;
			}
			
			lcd_stringout(messages[messageIndex]);
			
			_delay_ms(150);
		}

        // Down button pressed
		if((adc_result < 110) && (adc_result > 90)){
			lcd_moveto(0,0);
			
			//Cycles message to make sure it doesn't go out of bounds
			messageIndex = (messageIndex + 1) % 10;
			
			lcd_stringout(messages[messageIndex]);
			_delay_ms(150);
		}

        // Select button pressed
        if((adc_result < 215) && (adc_result > 195)){
			lcd_moveto(1,0);
			
			/* 	Sends current message out of current ardunio to
			*	other ardunio */
			serial_stringout(messages[messageIndex]);
			_delay_ms(150);
		}

        // Message received from remote device
		if(finished == 1){
			buffIndex = 0; 			//Reset buffer index
			finished = 0;			//Reset finished flag
			lcd_moveto(1,0);
			lcd_stringout(buffer);	//Prints out input to current ardunio
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
	DDRD |= (1 << PD3);					  // Enables PD3 as output
	PORTD &= ~( 1 << PD3 );				  // Sets PD3 to 0
	
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
	for(i = 0; i < 16; i++){
		serial_txchar(s[i]);
	}
    

}

/* Interrupt when recieving a serial input*/
ISR(USART_RX_vect)
{

    // Handle received character
	char ch;
	
	ch = UDR0;		// Get the received charater
	
	
	// Store in buffer and increment bufferIndex
	buffer[buffIndex++] = ch;


    // If message complete, set flag
	if(buffIndex == 16){
		finished = 1;
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