/********************************/
/*          GUI lab             */
/********************************/


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "library.h"

#define LEDPort 0x32		/* LED port */
#define LCDPort 0x33		/* LCD port */
#define KbdPort 0x34		/* keyboard port */


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
  0x00, 0x18, 0x08, 0x03};

unsigned char ScanCode;

#define Col7Lo 0xF7            // column 7 scan
#define Col6Lo 0xFB            // column 6 scan
#define Col5Lo 0xFD            // column 5 scan
#define Col4Lo 0xFE            // column 4 scan

static void initlcd();
static void lcd_writecmd(char cmd);
static void LCDprint(char *sptr);
static void lcddata(unsigned char cmd);

int main(int argc, char *argv[])
{
	system("killall pqiv");												// close previous instances of PQIV if any
	CM3DeviceInit();
	unsigned char key; 													// store last key pressed
	
	initlcd();															// initialise LCD
	lcd_writecmd(0x80);													// set LCD to line 1
	LCDprint("---ATM Demo---");											// output string to LCD	
	lcd_writecmd(0xC0);													// set LCD to line 2
	LCDprint("Follow screen...");										// output to LCD
	system("DISPLAY=:0.0 pqiv -f /tmp/welcome.jpg &");       			// on main display, launch PQIV, fulscreen, image path in linux, continue program

	while(1)															// loop forever
	{
		unsigned char i,ii;
		i = ScanKey();

		if (i != 0xFF)													// if key is pressed
		{
			key = ScanKey();											// store last key pressed
		}

		if (i != 0xFF)
		{
			if (i > 0x39) {												// if numerals pressed output numbers, if * or # pressed output A and B
				ii = i - 0x37;
			} else {
				ii = i - 0x30;
			}

			CM3_outport(LEDPort, Bin2LED[ii]);							// output keypress to LED
		}


		if (key == '2')													// if key 2 is pressed
		{
			system("DISPLAY=:0.0 pqiv -f /tmp/amount.jpg &");			// launch image
			while (ScanKey() != '4','5') 								// loop while waiting for 4 or 5 to be pressed
			{
			
				if (ScanKey() == '4')									// if key 4 is pressed
				{
					system("DISPLAY=:0.0 pqiv -f /tmp/20.jpg &");		// launch image
					while(ScanKey() !='3');                            	// loop while waiting for 3 to be pressed
					key = '3';
					break;												// exit loop
				}
				if (ScanKey() == '5')									// if key 4 is pressed
				{
					system("DISPLAY=:0.0 pqiv -f /tmp/50.jpg &");		// launch image
					while(ScanKey() !='3');								// loop while wailing for 3 to be pressed
					key = '3';
					break;												// exit loop
				}
			}
		} 

		if (key == '3')													// if key 3 was pressed
		{
			system("DISPLAY=:0.0 pqiv -f /tmp/thankyou.jpg &");			// launch image
			while(ScanKey() !='1');										// loop while waiting for 1 to be pressed
			system("killall pqiv"); 									// clear all instances of pqiv
			system("DISPLAY=:0.0 pqiv -f /tmp/welcome.jpg &");        	// launch starting image
			key = 0xff;                                                 // reset key to 0 (not pressed)
		}

		usleep(300000);
		

	}

	CM3DeviceDeInit();   

}  

//----------- LCD Functions --------------

static void initlcd(void)
{
    usleep(20000);
	lcd_writecmd(0x30);
    usleep(20000);
	lcd_writecmd(0x30);   
  	usleep(20000);
	lcd_writecmd(0x30);

	lcd_writecmd(0x02);  // 4 bit mode 
	lcd_writecmd(0x28);  // 2 line  5*7 dots
	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x0c);  //dis on cur off
	lcd_writecmd(0x06);  //inc cur
	lcd_writecmd(0x80);
}

static void lcd_writecmd(char cmd)
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

static void LCDprint(char *sptr)
{
	while (*sptr != 0)
	{
		int i=1;
        lcddata(*sptr);
		++sptr;
	}
}

static void lcddata(unsigned char cmd)
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
	CM3_outport(KbdPort, Col7Lo);
	ScanCode = CM3_inport(KbdPort);
	ScanCode |= 0x0F;
	ScanCode &= Col7Lo;
	if (ScanCode != Col7Lo)
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
	unsigned char j;
	for (j = 0 ; j <= 12 ; j++)
	if (ScanCode == ScanTable [j])
	{
	   if(j > 9) {
		   j = j + 0x37;
	   } else {
		   j = j + 0x30;
	   }
	   return j;
	}

	if (j == 12)
	{
		return 0xFF;
	}

	return (0);
}

