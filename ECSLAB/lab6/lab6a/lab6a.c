/********************************/
/*          DAC lab             */
/********************************/

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "library.h"

#define DACPort 0x30

unsigned char i;
unsigned char j;
unsigned char reverse(unsigned char);

unsigned char WaveTable [] = {___,___,___,___,___,___,___,___,___,___,___,___};

/************ MAIN PROGRAM ****************/

int main(int args,char* argv[])

{
	CM3DeviceInit();															
	printf("DAC Test\n");
	printf("Connect Pin 3 and 2 of selection jumper Connector J3\n");

    i = 0;
	while(1)
	{
	        j = reverse(WaveTable[i]);	/* reverse bits before output */
		CM3_outport(DACPort, j);	/* output to DAC */
		i++;
		if (i >= 12)			/* repeat at 12 */
		{                  
			i=0;
		}
		usleep(20);	
	}
	CM3DeviceDeInit();
}
unsigned char reverse(unsigned char num)	/* function to reverse bits - to remove*/
{
	unsigned char NO_OF_BITS = sizeof(num) * 8;
	unsigned char reverse_num = 0;
	int i;
	for (i = 0; i < NO_OF_BITS; i++)
	{
		if((num & (1 << i))) {
			reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
		}
	}
	return reverse_num;
}


