volatile char buffer[5];		// Holds the messages
volatile int buffIndex;			// Holds the index of the current
								// char incoming for buffer
volatile int finished ;			// If the array is finished processing
								// this flag is set to one