extern volatile char buffer[5];		// Holds the messages
extern volatile int buffIndex;		// Holds the index of the current
									// char incoming for buffer
extern volatile int finished ;		// If the array is finished processing
									// this flag is set to one