/*******************************/
/*    LCD & Keypad interface   */
/*******************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "library.h"


#define LEDPort 0x3A		/* LED port */
#define LCDPort 0x3B			/* LCD port */
#define SMPort 0x39 	    // stepper motor port
#define KbdPort 0x3C			/* Keypad port */

#define	NumSteps	200
#define	PtableLen	4
#define AUDIOFILE1 "/tmp/ecs_slide1_check_car.raw"
#define AUDIOFILE2 "/tmp/ecs_slide2_open_gantry_entry.raw"
#define AUDIOFILE3 "/tmp/ecs_slide3_request_ticket.raw"
#define AUDIOFILE4 "/tmp/ecs_slide4_pay_parking.raw"
#define AUDIOFILE5 "/tmp/ecs_slide5_exit_success.raw"


unsigned char Ptable []={0x01, 0x02, 0x4, 0x08};


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

static void displaycurrentime();
static void openGantry();
static void printTicket();
static int guichange(int selectant);




static void initlcd();
static void lcd_writecmd(char cmd);
static void LCDprint(char *sptr);
static void lcddata(unsigned char cmd);
static void moveMotor(int direction);
static void runDAC(int i);



/************* MAIN PROGRAM ******************/

int main(int argc, char *argv[])
{
	CM3DeviceInit();
	CM3PortInit(5);  // Initialise DAC
	
	initlcd();
	sleep(1);
//	runDAC(1); 
	usleep(3000000);



	while(1)
	{
		unsigned char i,ii;
		unsigned char key;
		
		static int car_status,gui_status,gui_initial_status=0;

		time_t entryTime, exitTime;

		if(gui_initial_status==0){             //prevents constant spamming of gui change
			gui_initial_status=guichange(1);
		}

		displaycurrentime();

		i = ScanKey();
		
		if (i != 0xFF)									// if key is pressed
		{
			if (i > 0x39) {                             // if numerals pressed output numbers, if * or # pressed output A and B
				ii = i - 0x37;
			} else {
				ii = i - 0x30;
			}

			if (ii == 1) { //press 1 to simulate car detected

				lcd_writecmd(0x01);  //clear screen
				lcd_writecmd(0x80);
				LCDprint("Car Detected");
				usleep(2000000);

				

				if(car_status==0){ //if no car inside carpark, enter this into carlist
				    openGantry();
					car_status=1;
					entryTime=time(NULL);
					gui_initial_status=0;
					guichange(2);
//					runDAC(2); 
					usleep(3000000);
				}else if(car_status==1){ //if car is inside carpark, print ticket
				    guichange(3);
//					runDAC(3); 
					usleep(3000000);
					lcd_writecmd(0x01);  
				    LCDprint("Press # for");
				    lcd_writecmd(0xC0);
				    LCDprint("Ticket");
					car_status=0;
					key = '5';
					while (key != 'B'){
					if (i != 0xFF)													// if key is pressed
					{ 
						key = ScanKey();											// store last key pressed
					}	
					if (key == 'B'){
						exitTime=time(NULL);
						double paymentamount = (difftime(exitTime, entryTime))/60*0.2;
						printTicket(paymentamount);
						gui_initial_status = 0;
						break;
					}
				}
				}


			}

            //what is this for?
			//lcddata(i);                                 // output to LCD
			CM3_outport(LEDPort, Bin2LED[ii]);			// output to LED
			usleep(1000000); //sleep for 1 seconds
		}
	}

	CM3DeviceDeInit();
}
static int guichange(int selectant) {
    // Close previous instances of PQIV if any
    system("killall pqiv");

    // Variable to hold the return value
    int result = 0;

    // Switch case based on the value of selectant
    switch (selectant) {
        case 1:
            system("DISPLAY=:0.0 pqiv -f /tmp/slide1_check_car.jpg &"); // welcome screen
            result = 1;
            break;
        case 2:
            system("DISPLAY=:0.0 pqiv -f /tmp/slide2_entry_recorded.jpg &"); // entry recorded screen
            result = 2;
            break;
        case 3:
            system("DISPLAY=:0.0 pqiv -f /tmp/slide3_get_ticket.jpg &"); // exit ticket screen
            result = 3;
            break;
        case 4:
            system("DISPLAY=:0.0 pqiv -f /tmp/slide4_pay_parking.jpg &"); // paying screen
            result = 4;
            break;
        case 5:
            system("DISPLAY=:0.0 pqiv -f /tmp/slide5_exit_success.jpg &"); // goodbye screen
            result = 5;
            break;
        default:
            result = 0;
            break;
    }

    return result;
}

static void printTicket(double paymentamount){

    // Display the formatted time on the LCD
	char display[10];
	displaycurrentime();
	sprintf (display, "$%.2f", paymentamount);

    // Show "8" on the 7-segment LED
    CM3_outport(LEDPort, Bin2LED[8]);

	 // Convert payment amount to string for display
    char amountStr[16];
    snprintf(amountStr, sizeof(amountStr), display);

	//beep alert sound
	lcd_writecmd(0xC0);
	LCDprint(amountStr);
	guichange(4);
//	runDAC(4); 
	usleep(3000000);
	openGantry();
	guichange(5);
	runDAC(5); 
	usleep(3000000);

}
static void displaycurrentime(void){
	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x80);
	time_t now;
	struct tm*tm_info;
	char timestring[16];
	time(&now);
	tm_info = localtime(&now);
	strftime(timestring,sizeof(timestring),"%H:%M:%S",tm_info);
	LCDprint(timestring);
	usleep(200000);
}





static void openGantry(void){
	unsigned char i;
	//open gantry
	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x80);

	LCDprint("Gantry Opened");
	moveMotor(1);
	usleep(5000000);
	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x80);

	i = ScanKey();

	while (i != 0xFF){
		i = ScanKey();
	}

	LCDprint("Gantry Closing");
	moveMotor(0);
	guichange(3);

	lcd_writecmd(0x01);  //clear screen
	lcd_writecmd(0x80);
	LCDprint("Have A");
	lcd_writecmd(0xC0);
	LCDprint("Nice Day");
	
}


static void moveMotor(int direction) {
	int k,j;
	k=0;
	if (direction == 1) { //normal
		for (j=NumSteps;j>0;j--)
		{
			CM3_outport(SMPort, Ptable[k]);	/* output to stepper motor */
			usleep(10000);                  /* delay */
			k++;
			if (k>=PtableLen) k=0;
		}
	} else { //reverse
		for (j=NumSteps;j>0;j--)
		{
			CM3_outport(SMPort, Ptable[k]);	/* output to stepper motor */
			usleep(10000);                  /* delay */
			k--;
			if (k<0) k=PtableLen-1;
		}
	}
	
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
		int k=1;
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
void runDAC(int i) {
    unsigned char buffer[1];
    FILE *ptr;
	
	switch(i){
		case 1: 
			ptr = fopen(AUDIOFILE1, "rb");
			if (ptr == NULL) {
				perror(AUDIOFILE1);
				printf("File cannot be found\n");
				return;
			}
			break;
		case 2:
			ptr = fopen(AUDIOFILE2, "rb");
			if (ptr == NULL) {
				perror(AUDIOFILE2);
				printf("File cannot be found\n");
				return;
			}
			break;
		case 3:
			ptr = fopen(AUDIOFILE3, "rb");
			if (ptr == NULL) {
				perror(AUDIOFILE3);
				printf("File cannot be found\n");
				return;
			}
			break;
		case 4:
			ptr = fopen(AUDIOFILE4, "rb");
			if (ptr == NULL) {
				perror(AUDIOFILE4);
				printf("File cannot be found\n");
				return;
			}
			break;
		case 5:
			ptr = fopen(AUDIOFILE5, "rb");
			if (ptr == NULL) {
				perror(AUDIOFILE5);
				printf("File cannot be found\n");
				return;
			}
			break;
	}


    // Open the audio file for reading and quit on error
    

    // Track the start time
    clock_t start_time = clock(),current_time;
    double elapsed_time;

    // Read from the file and write to the DAC
    while (fread(buffer, sizeof(buffer), 1, ptr) == 1) {
        // Check elapsed time
        current_time = clock();
        elapsed_time = (double)(current_time - start_time) / CLOCKS_PER_SEC;
        if (elapsed_time > 1.0) {  // 5 seconds limit
            printf("DAC operation limited to 5 seconds.\n");
            break;
        }

        CM3PortWrite(3, buffer[0]);
        usleep(100);  // Adjust sleep time as needed
    }

    // Close the file
    fclose(ptr);
}
