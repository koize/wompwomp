/*******************************/
/*    LCD & Keypad interface   */
/*******************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "library.h"

#define LEDPort 0x__			/* LED port */
#define LCDPort 0x__			/* LCD port */
#define KbdPort 0x__			/* Keypad port */


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

const unsigned char Bin2LED[] =                           
    /*  0     1     2    3 */
    {0x__, 0x__, 0x__, 0x__,
    /*  4     5     6    7*/
    0x__, 0x__, 0x__, 0x__,
    /*  8     9     A    B*/
    0x__, 0x__, 0x__, 0x__,
    /*  C     D     E    F*/
    0x__, 0x__, 0x__, 0x__};


unsigned char ScanCode;			// hold scan code returned

#define Col7Lo 0x__            	// column 7 scan
#define Col6Lo 0x__           	// column 6 scan
#define Col5Lo 0x__            	// column 5 scan
#define Col4Lo 0x__            	// column 4 scan

static void initlcd();
static void lcd_writecmd(char cmd);
static void LCDprint(char *sptr);
static void lcddata(unsigned char cmd);

/************* MAIN PROGRAM ******************/

int main(int argc, char *argv[])
{
	CM3DeviceInit();
	
	initlcd();
	sleep(1);
	lcd_writecmd(0x80);
	LCDprint("LCD Lab");
	lcd_writecmd(0xC0);
	LCDprint("12345678");

	while(1)
	{
		unsigned char i,ii;
		i = ScanKey();
		if (i != 0xFF)									// if key is pressed
		{
			if (i > 0x39) {                             // if numerals pressed output numbers, if * or # pressed output A and B
				ii = i - 0x37;
			} else {
				ii = i - 0x30;
			}

			lcddata(i);                                 // output to LCD
			CM3_outport(LEDPort, Bin2LED[ii]);			// output to LED
			usleep(300000);
		}
	}

	CM3DeviceDeInit();   

}  

static void initlcd(void)                               // function to initialise LCD
{
	usleep(20000);
	lcd_writecmd(0x30);									// Initialise
	usleep(20000);
	lcd_writecmd(0x30);   
  	usleep(20000);
	lcd_writecmd(0x30);

	lcd_writecmd(____);  								// return cursor to home; return display to orig position 
	lcd_writecmd(____);  								// 4 bit mode, 2 line,  5*7 dots
	lcd_writecmd(____);  								// clear screen
	lcd_writecmd(____);  								// dis on cur off
	lcd_writecmd(____);  								// inc cur
	lcd_writecmd(____);								// line 1
}

static void lcd_writecmd(char cmd)                      // function to output hex as commands to LCD display
{
	char data;

	data = (cmd & 0xf0);
	CM3_outport(LCDPort, data | 0x04);
	usleep(10);
	CM3_outport(LCDPort, data);

	usleep(200);

	data = (cmd & 0x0f) << 4;
	CM3_outport(LCDPort, data | 0x04);
	usleep(10);
	CM3_outport(LCDPort, data);

	usleep(2000);
}

static void LCDprint(char *sptr)                        // function to print string to LCD
{
	while (*sptr != 0)
	{
		int i=1;
        lcddata(*sptr);
		++sptr;
	}
}

static void lcddata(unsigned char cmd)					// fuction to print single int or char to LCD
{

	char data;

	data = (cmd & 0xf0);
	CM3_outport(LCDPort, data | 0x05);
	usleep(10);
	CM3_outport(LCDPort, data);

	usleep(200);

	data = (cmd & 0x0f) << 4;
	CM3_outport(LCDPort, data | 0x05);
	usleep(10);
	CM3_outport(LCDPort, data);

	usleep(2000);
}

//----------- Keypad Functions ----------------

unsigned char ScanKey()
{
	CM3_outport(KbdPort, Col7Lo);					// bit 7 low 
	ScanCode = CM3_inport(KbdPort);					// Read
	ScanCode |= 0x0F;								// high nybble to 1
	ScanCode &= Col7Lo;								// AND back scan value
	if (ScanCode != Col7Lo)							// in <> out get key and display
	{
	    return ProcKey();
	}

	CM3_outport(KbdPort, Col6Lo);
	ScanCode = CM3_inport(KbdPort);
	ScanCode |= 0x0F;
	ScanCode &= Col6Lo;
	if (ScanCode != Col6Lo)
	{
	    return ProcKey();
	}

	CM3_outport(KbdPort, Col5Lo);
	ScanCode = CM3_inport(KbdPort);
	ScanCode |= 0x0F;
	ScanCode &= Col5Lo;
	if (ScanCode != Col5Lo)
	{
	    return ProcKey();
	}

	CM3_outport(KbdPort, Col4Lo);
	ScanCode = CM3_inport(KbdPort);
	ScanCode |= 0x0F;
	ScanCode &= Col4Lo;
	if (ScanCode != Col4Lo)
	{
	    return ProcKey();
	}

	return 0xFF;
}

unsigned char ProcKey()
{
	unsigned char j;								// index of scan code returned
	for (j = 0 ; j <= 12 ; j++)
	if (ScanCode == ScanTable [j])					// search in table
	{
	   if(j > 9) {
		   j = j + 0x37;
	   } else {
		   j = j + 0x30;
	   }
	   return j;                           			// exit loop if found
	}

	if (j == 12)
	{
		return 0xFF;                           		// if not found, return 0xFF
	}

	return (0);
}
