/*****************************/
/*          LED lab          */
/*****************************/
#include<stdio.h>
#include<unistd.h>
#include "library.h"

#define LEDPort 0x32

const unsigned char Bin2LED[] = 
                           
    /*  0     1     2    3 */
    {0x__, 0x__, 0x__, 0x__,
    /*  4     5     6    7*/
    0x__, 0x__, 0x__, 0x__,
    /*  8     9     A    B*/
    0x__, 0x__, 0x__, 0x__,
    /*  C     D     E    F*/
    0x__, 0x__, 0x__, 0x__};

/***** MAIN PROGRAM *****/   
                      
int main(int argc, char *argv[])
{

	int LEDval;
   	CM3DeviceInit();

    for(int i=0;i<16;i++)   // only 1 to 15
	{
                              	
        LEDval = Bin2LED[i];	
                      
		CM3_outport(LEDPort,LEDval);    // output to LED

		printf(" %d \n",i);
                
		sleep(1);	  // non blocking sleep

	}
	
   	CM3DeviceDeInit();
}  
