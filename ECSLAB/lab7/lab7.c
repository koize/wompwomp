/************************************/
/*         Graphics lab             */
/************************************/


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "library.h"

#define LEDPort 0x32 		/* LED port */
#define LCDPort 0x33		/* LCD port */
#define KbdPort 0x34		/* keypad port */


unsigned char ProcKey();
unsigned char ScanKey();

const unsigned char ScanTable [12] =
{
  /* 0     1     2     3 */
     0xB7, 0x7E, 0xBE, 0xDE,
  /* 4     5     6     7 */
   0x7D, 0xBD, 0xDD, 0x7B,
  /* 8     9     *     # */
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

/*************** MAIN PROGRAM ******************/

int main(int argc, char *argv[])
{
	system("killall pqiv");													// clear any instances of pqiv if any
	CM3DeviceInit();
	
	initlcd();
	lcd_writecmd(0x80);                 /* set LCD to line 1 */
	LCDprint("Press 1 / 2 / 3"); 		/* print string to LCD */
	lcd_writecmd(0xC0);					/* set LCD to line 2 */
	LCDprint("(4 to Exit)");			/* print string to LCD */

	while(1)                            /* loop forever */
	{
		unsigned char i,ii;													// store keypad input and modified keypad input
		i = ScanKey();
		if (i != 0xFF)														// if key is pressed
		{
			if (i > 0x39) {
				ii = i - 0x37;
			} else {
				ii = i - 0x30;
			}

			CM3_outport(LEDPort, Bin2LED[ii]);    							// output to LED

			if (i == '1')
			{
				system("DISPLAY=:0.0 pqiv -f /tmp/colour.jpg &");    		// linux system command to launch PQIV ("specify display, launch PQIV, launch fullscreen, image path in linux, continue with program")
			} 																//									   ("DIAPLAY=:0.0        pqiv           -f              /tmp/image1/jpg            &            ")      
			if (i == '2')
			{
				system("DISPLAY=:0.0 pqiv -f /tmp/esplanade.jpg &");
			}
			if (i == '3')
			{
				system("DISPLAY=:0.0 pqiv -f /tmp/sunset@paya.jpg &");
			}
			if (i == '4')
			{
				lcd_writecmd(0x01);			/* clear screen */
				LCDprint("      B  Y  E  ");	/* print string to LCD */
				system("killall pqiv");     /* close all instances of PQIV */
				exit(0);					/* exit program */
			}

			usleep(300000);
		}
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
