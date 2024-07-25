#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "library.h"
#include "wiringPi.h"
#include <signal.h>
#include <pthread.h>

#define LEDPort 0x3A
#define KbdPort 0x3C
#define LCDPort 0x3B
#define SMPort  0x39

#define Col7Lo 0xF7            // column 7 scan
#define Col6Lo 0xFB            // column 6 scan
#define Col5Lo 0xFD            // column 5 scan
#define Col4Lo 0xFE            // column 4 scan

#define AUDIOFILE "/tmp/shootingstars.raw"

int i,reading,h;
char inp;
int full_seq_drive[4] = {0x08, 0x04, 0x02, 0x01};

int anti_clockwise[4][4] = {
	{0,0,0,1},
	{0,0,1,0},
	{0,1,0,0},
	{1,0,0,0}
};

const unsigned char Bin2LED[] =
{ 
	/* 0     1     2    3 */
	0x40, 0x79, 0x24, 0x30,
	/*  4     5     6    7*/
	0x19, 0x12, 0x02, 0x78,
	/*  8     9     A    B*/
	0x00, 0x18, 0x08, 0x03,
	/*  C     D     E    F*/
	0x46, 0x21, 0x06, 0x0E
};

const unsigned char ScanTable [12] =
{
  // 0     1     2     3	
	0xB7, 0x7E, 0xBE, 0xDE,
  // 4     5     6     7	
	0x7D, 0xBD, 0xDD, 0x7B,
  // 8     9     *     #
	0xBB, 0xDB, 0x77, 0xD7
};

unsigned char dac_start[] = {"Running DAC"};
unsigned char dac_stop[] = {"Stopping DAC"};
unsigned char motor_start[] ={"Starting motor"};
unsigned char motor_stop[] ={"Stopping motor"};
unsigned char stop[] ={"Exiting Lab"};
unsigned char image[]={"Image Slideshow"};


// Declaration of thread condition variable 
pthread_t id[3];
//unsigned int start_motor = 0;
//unsigned int stop_motor = 0;
//unsigned int start_dac = 0;
//unsigned int stop_adc = 0;

pthread_t motor_id;
pthread_t dac_id;
// declaring mutex 
pthread_mutex_t bus_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t motorlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t daclock = PTHREAD_MUTEX_INITIALIZER;  

pthread_cond_t t2 = PTHREAD_COND_INITIALIZER; 
pthread_cond_t t3 = PTHREAD_COND_INITIALIZER;

void initlcd();
void lcd_writecmd(char cmd);
void LCDprint(char *sptr);
void lcddata(unsigned char cmd);

unsigned char ProcKey();
unsigned char ScanKey();
unsigned char ScanCode;

void* thread_dac(void* value)
{
	dac_id = pthread_self();

	unsigned char data[2];
	unsigned short fn[100];
	float gain= 255.0f;
	float phase = 0.0f;
	float bias = 255.0f;//1024.0f;
	float freq = 2.0* (3.14159f) /4.0;
	unsigned char buffer[1];
	int fileend;
	FILE *ptr;
	ptr = fopen(AUDIOFILE,"rb");

	CM3DeviceInit();															
	CM3PortInit(5);																// initialise DAC
	printf("Connect Pin 1 and 2 of selection jumper Connector J3\n");

    if (( ptr = fopen(AUDIOFILE, "r")) == NULL)
	{
		perror (AUDIOFILE);
		printf ("File cannot be found \n");
		return (0);
	}

    while ( (fileend = fgetc(ptr) ) != EOF) 
	{
	    fread(buffer,sizeof(buffer),2,ptr);
	    for(int i=0; i<1 ; i++) {
			CM3PortWrite(3, buffer[i]);
			//usleep(1);
	    }


			//}
		//}
		pthread_mutex_unlock(&daclock);
		//usleep(100);
	}
}

void* thread_motor(void* value)
{   

	int j;    

	motor_id = pthread_self();

	while(1)
	{ 
		pthread_mutex_lock(&motorlock);
		{
			for(i=0;i<4;i++)
			{
				pthread_mutex_lock(&bus_lock);
				CM3_outport(SMPort, full_seq_drive[i]);
				pthread_mutex_unlock(&bus_lock);
				usleep(8000);
			}
		}	
		pthread_mutex_unlock(&motorlock); 
		usleep(100);
	}	
}




void* thread_keypad(void* value)
{
	unsigned char i;
	int mutex_check;
	int mutex_check_dac;

	pthread_mutex_lock(&motorlock);
	pthread_mutex_lock(&daclock);

	while(1)
	{
		pthread_mutex_lock(&bus_lock);
		i = ScanKey();
		pthread_mutex_unlock(&bus_lock);
		usleep(100000);

		if (i == '3' )
		{
			printf("Pressed Key: %c\n",i);
			initlcd();
			int mj = 0;
				
			pthread_mutex_lock(&bus_lock);
			LCDprint("Starting motor");
			pthread_mutex_unlock(&bus_lock);
				
			usleep(1000);
			pthread_mutex_lock(&bus_lock);
			CM3_outport(LEDPort,Bin2LED[3]);
			pthread_mutex_unlock(&bus_lock);

			pthread_mutex_unlock(&motorlock);					// unlock mutex in motor thread
			mutex_check = 1;

			usleep(10000);
			printf("Motor Started\n");
		}

		if (i == '4')
		{
			usleep(10000);
			if (mutex_check == 1)								// check so lock doesnt repeat and loop program
			{
				pthread_mutex_lock(&motorlock); 				// lock mutex of motor thread
			}						
			mutex_check = 0;

			usleep(100);
			printf("Pressed Key: %c\n",i);
			initlcd();
			int msj = 0;
			
			pthread_mutex_lock(&bus_lock);
			LCDprint("Stopping motor");
			pthread_mutex_unlock(&bus_lock);

			usleep(1000);
			pthread_mutex_lock(&bus_lock);
			CM3_outport(LEDPort,Bin2LED[4]);
			pthread_mutex_unlock(&bus_lock);
			usleep(10000);
			printf("Motor Stopped\n");
		}

		if(i == '1')
		{
			printf("Pressed Key: %c\n",i);
			initlcd();
			int j = 0;
			
			pthread_mutex_lock(&bus_lock);
			LCDprint("Running DAC");
			pthread_mutex_unlock(&bus_lock);

			usleep(1000);
			printf("Playing Music \n");
			pthread_mutex_lock(&bus_lock);
			CM3_outport(LEDPort,Bin2LED[1]);
			pthread_mutex_unlock(&bus_lock);
			usleep(10000);

			pthread_mutex_unlock(&daclock);
			mutex_check_dac = 1;
			//start_dac = 1;
			//stop_adc = 0;
		}

		if(i == '2')
		{
			//start_dac = 0;
			//stop_adc = 1;
			if (mutex_check_dac == 1)								
			{
				pthread_mutex_lock(&daclock); 				
			}						
			mutex_check_dac = 0;

			printf("Pressed Key: %c\n",i);
			initlcd();
			int sj = 0;
			
			pthread_mutex_lock(&bus_lock);
			LCDprint("Stopping DAC");
			pthread_mutex_unlock(&bus_lock);

			usleep(1000);

			pthread_mutex_lock(&bus_lock);
			usleep(10000);
			CM3_outport(LEDPort,Bin2LED[2]);
			pthread_mutex_unlock(&bus_lock);
			printf("Music Stopped \n");
		}

		if(i == '6')
		{
			printf("Program Terminated \n");
			initlcd();
			int tj = 0;
			int LCDValt;	
			
			pthread_mutex_lock(&bus_lock);
			LCDprint("Exiting Lab");
			pthread_mutex_unlock(&bus_lock);

			usleep(1000);
			pthread_mutex_lock(&bus_lock);
			usleep(10000);
			CM3_outport(LEDPort,Bin2LED[6]);
			pthread_mutex_unlock(&bus_lock);
			system("killall pqiv");	
			exit(0); 


		}

		if(i == '5')
		{
			initlcd();
			int ssj = 0;
			int LCDValss;	
			
			pthread_mutex_lock(&bus_lock);
			LCDprint("Image Slideshow");
			pthread_mutex_unlock(&bus_lock);

			usleep(1000);

			pthread_mutex_lock(&bus_lock);
			usleep(10000);
			CM3_outport(LEDPort,Bin2LED[5]);
			pthread_mutex_unlock(&bus_lock);

			pid_t pid;
			pid =fork();
			if(pid==0){
				setsid();

				system("DISPLAY=:0.0 pqiv -f -s --slideshow-interval=0.5 /tmp/demo.jpg /tmp/slide.jpg &");
				return 0;
			}
			else{

				while(1){
					pthread_mutex_lock(&bus_lock);
					i = ScanKey();
					pthread_mutex_unlock(&bus_lock);
					usleep(10000);
					if(i == '7')
					{  
						killpg(pid,SIGKILL);
						printf("Closing slide \n");
						break;
					}
				}

			}  

		}


	}
}



int main(int agrv,char* argc[])
{
	int* ptr;
    system("killall pqiv");
	system("DISPLAY=:0.0 pqiv -f /tmp/demo.jpg &");
	sleep(2);
	CM3DeviceInit();
	CM3DeviceSpiInit(0);


	pthread_create(&id[0],NULL,thread_keypad,NULL);
	pthread_create(&id[1],NULL,thread_motor,NULL);
	pthread_create(&id[2],NULL,thread_dac,NULL);

	//     }


	//CM3DeviceDeInit();
	pthread_join(id[0], (void**)&ptr);
	pthread_join(id[1], (void**)&ptr);
	pthread_join(id[2], (void**)&ptr);
	return 0;
}

//----------- LCD Functions ----------------

void initlcd(void)
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

void lcd_writecmd(char cmd)
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

void LCDprint(char *sptr)
{
	while (*sptr != 0)
	{
		int i=1;
        lcddata(*sptr);
		++sptr;
	}
}

void lcddata(unsigned char cmd)
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




