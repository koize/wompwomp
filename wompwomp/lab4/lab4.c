/*******************************/
/*    LCD & Keypad interface   */
/*******************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "library.h"

#define LEDPort 0x3A		/* LED port */
#define LCDPort 0x3B			/* LCD port */
#define KbdPort 0x34			/* Keypad port */


unsigned char ProcKey();
unsigned char ScanKey();

const unsigned char ScanTable [12] =
{
  // 0     1     2     3	
	0xB7, 0x7E, 0xBE, 0xDE,
  // 4     5     6     7	
	0x7D, 0xBD, 0xDD, 0x7B,
  // 8     9     *     #
	0xBB, 0xDB, 0x77, 0xD7
};

const unsigned char Bin2LED[] = 
    /*  0     1     2    3 */
    {0x40, 0x79, 0x24, 0x30,
    /*  4     5     6    7*/
    0x19, 0x12, 0x02, 0x78,
    /*  8     9     A    B*/
    0x00, 0x18, 0x08, 0x03,
	/*  C     D     E    F*/
    0x46, 0x21, 0x06, 0x0E};


unsigned char ScanCode;			// hold scan code returned

#define Col7Lo 0xF7            // column 7 scan
#define Col6Lo 0xFB            // column 6 scan
#define Col5Lo 0xFD            // column 5 scan
#define Col4Lo 0xFE            // column 4 scan

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
	lcd_writecmd(0x02);  // 4 bit mode 
	lcd_writecmd(0x28);  // 2 line  5*7 dots
	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x0c);  //dis on cur off
	lcd_writecmd(0x06);  //inc cur
	lcd_writecmd(0x80);							// line 1
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
