/* ***************************************************** */
/* SETR 21/22, Paulo Pedreiras                           */
/* Base code for Unit Testing                            */
/*    Simple example of command processor                */
/*    Note that there are several simplifications        */
/*    E.g. Kp, Ti and Td usually are not integer values  */
/*    Code can (and should) be improved. E.g. error      */ 
/*        codes are "magic numbers" in the middle of the */
/*        code instead of being (defined) text literals  */
/* ***************************************************** */

#include <stdio.h>

#include "cmdproc.h"

/* PID parameters */
/* Note that in a real application these vars would be extern */
char Kp, Ti, Td;

/* Process variables */ 
/* Note that in a real application these vars would be extern */
int setpoint, output, error; 

/* Internal variables */
static char cmdString[MAX_CMDSTRING_SIZE];
static unsigned char cmdStringLen = 0; 
char CS;

/* ************************************************************ */
/* Processes the the chars received so far looking for commands */
/* Returns:                                                     */
/*  	 0: if a valid command was found and executed           */
/* 	-1: if empty string or incomplete command found             */
/* 	-2: if an invalid command was found                         */
/* 	-3: if a CS error is detected (command not executed)        */
/* 	-4: if string format is wrong                               */
/* ************************************************************ */
int cmdProcessor(void)
{
	int i,j;
	
	/* Detect empty cmd string */
	if(cmdStringLen == 0)
		return -1; 
	
	/* Find index of SOF */
	for(i=0; i < cmdStringLen; i++) {
		if(cmdString[i] == SOF_SYM) {
			break;
		}
	}

	/* Find index of EOF_SYM */
	for(j=i; j < cmdStringLen; j++) {
		if(cmdString[j] == EOF_SYM) {
			break;
		}
	}

	/* If EOF_SYM not found return  -4 */
	if(j == cmdStringLen)
		return -4;

	/* If a SOF was found look for commands */
	if(i < cmdStringLen) {
		if(cmdString[i+1] == 'P') { /* P command detected */
			
			/* If checksum error return -3 */
			CS = cmdString[i+1] + cmdString[i+2] + cmdString[i+3] + cmdString[i+4];
			
			if(CS != cmdString[i+5])
				return -3;
			
			Kp = cmdString[i+2];
			Ti = cmdString[i+3];
			Td = cmdString[i+4];
			resetCmdString();
			return 0;
		}
		
		if(cmdString[i+1] == 'S') { /* S command detected */

			/* If checksum error return -3 */
			CS = cmdString[i+1];
			
			if(CS != cmdString[i+2])
				return -3;
			
			printf("Setpoint = %d, Output = %d, Error = %d", setpoint, output, error);
			resetCmdString();
			return 0;
		}

		/* Invalid Command */
		return -2;	
	}
	
	/* cmd string not null and SOF not found */
	return -4;

}

/* ******************************** */
/* Adds a char to the cmd string 	*/
/* Returns: 				        */
/*  	 0: if success 		        */
/* 		-1: if cmd string full 	    */
/* ******************************** */
int newCmdChar(unsigned char newChar)
{
	/* If cmd string not full add char to it */
	if (cmdStringLen < MAX_CMDSTRING_SIZE) {
		cmdString[cmdStringLen] = newChar;
		cmdStringLen +=1;
		return 0;		
	}
	
	/* If cmd string full return error */
	return -1;
}

/* ************************** */
/* Resets the commanbd string */  
/* ************************** */
void resetCmdString(void)
{
	cmdStringLen = 0;		
	return;
}
