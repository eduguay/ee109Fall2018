
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// Serial communications functions and variables
void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);


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