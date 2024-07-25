/**********************************/
/*      Stepper Motor lab         */
/**********************************/

#include <stdio.h>
#include <unistd.h>
#include "library.h"

#define SMPort 		0x31									// stepper motor port
#define	NumSteps	200
#define	PtableLen	4

unsigned char Ptable []={0x__,0x__,0x__,0x__};

/********** MAIN PROGRAM *************/

int main(int args, char *argv[])
{
	int i,j;
	CM3DeviceInit();

	printf("starting_stepper_motor\n");
	i=0;
	for (j=NumSteps;j>0;j--)
	{
		CM3_outport(SMPort, Ptable[i]);	/* output to stepper motor */
		usleep(10000);                  /* delay */
		i++;
		if (i>=PtableLen) i=0;
	}
	CM3DeviceDeInit();
}
