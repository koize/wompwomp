/**************************/
/*      Keypad lab        */
/**************************/

#include<stdio.h>
#include<unistd.h>
#include "library.h"

#define LEDPort 0x32			// LED Port
#define KbdPort 0x34			// Keypad port

#define Col7Lo 0x__            	// column 7 scan
#define Col6Lo 0x__           	// column 6 scan
#define Col5Lo 0x__            	// column 5 scan
#define Col4Lo 0x__            	// column 4 scan

const unsigned char Bin2LED[] =                         
    /*  0     1     2    3 */
    {0x__, 0x__, 0x__, 0x__,
    /*  4     5     6    7*/
    0x__, 0x__, 0x__, 0x__,
    /*  8     9     A    B*/
    0x__, 0x__, 0x__, 0x__,
    /*  C     D     E    F*/
    0x__, 0x__, 0x__, 0x__};

unsigned char ProcKey();
unsigned char ScanKey();

const unsigned char ScanTable [12] =
{
/* 0     1     2     3	*/
 0x__, 0x__, 0x__, 0x__,
/* 4     5     6     7 */	
 0x__, 0x__, 0x__, 0x__,
/* 8     9     *     # */
 0x__, 0x__, 0x__, 0x__
};

unsigned char ScanCode;									// hold scan code returned

/******** MAIN PROGRAM **********/
                             
int main(int argc, char *argv[])
{

	CM3DeviceInit();

	printf(" keypad_testing_on\n");

	while (1) {											// loop forever
		unsigned char i;
		unsigned char k;
        k = CM3_inport(KbdPort);
		i = ScanKey();                             

		if(i != 0xFF){									// if key is pressed
			printf("Key: %u pressed\n", i);
			CM3_outport(LEDPort, Bin2LED[i]);			// output to LED
		}
		usleep(150000);	 								// non blocking sleep
	}

   	CM3DeviceDeInit();	
}  

unsigned char ProcKey()
{
	unsigned char j;									//index of scan code returned
	for (j = 0 ; j <= 12 ; j++)	
	if (ScanCode == ScanTable [j])						//search in table
	{
	   return j; 										//exit loop if found
	}
	if (j == 12)
	{
		return 0xFF; 									// if not found, retun to 0xFF
	}
	return (0);
}

unsigned char ScanKey()
{
	CM3_outport(KbdPort, Col7Lo);						// bit 7 low
	ScanCode = CM3_inport(KbdPort);						// read
	ScanCode |= 0x0F;									// high nybble to 1
	ScanCode &= Col7Lo;                                 // AND back scan value
	if (ScanCode != Col7Lo)                             // in <> out get key

	{
	    return ProcKey();
	}

	CM3_outport(KbdPort, Col6Lo);						// bit 6 low
	ScanCode = CM3_inport(KbdPort);						// read
	ScanCode |= 0x0F;									// high nybble to 1
	ScanCode &= Col6Lo;									// AND back scan value
	if (ScanCode != Col6Lo)								// in <> out get key
	{

	    return ProcKey();
	}

	CM3_outport(KbdPort, Col5Lo);						// bit 5 low
	ScanCode = CM3_inport(KbdPort);						// read
	ScanCode |= 0x0F;									// high nybble to 1
	ScanCode &= Col5Lo;									// AND back scan value
	if (ScanCode != Col5Lo)								// in <> out get key
	{
	    return ProcKey();
	}

	CM3_outport(KbdPort, Col4Lo);						// bit 4 low
	ScanCode = CM3_inport(KbdPort);						// read
	ScanCode |= 0x0F;									// high nybble to 1
	ScanCode &= Col4Lo;									// AND back scan value
	if (ScanCode != Col4Lo)								// in <> out get key
	{
	    return ProcKey();
	}

	return 0xFF;
}


