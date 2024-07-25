/******************************************/
/*           Multitasking lab             */
/******************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "library.h"

#define LEDPort 0x3A													// LED port
#define SMPort 0x39 													// stepper motor port
#define LCDPort 0x3B													// LCD port

#define CW 0															// Clockwise
#define CCW 1															// Counter Clockwise

int motor_fast = 10000;													// Specify delay amount to vary speed of motor
int motor_slower = 20000;
int motor_slowest = 40000; 

int counter_clockwise[4] = {0x01, 0x04, 0x02, 0x08};
int clockwise[4] = {0x08, 0x02, 0x04, 0x01};

const unsigned char Bin2LED[] = 
    /*  0     1     2    3 */
    {0x40, 0x79, 0x24, 0x30,
    /*  4     5     6    7*/
    0x19, 0x12, 0x02, 0x78,
    /*  8     9     A    B*/
    0x00, 0x18, 0x08, 0x03,
	/*  C     D     E    F*/
    0x46, 0x21, 0x06, 0x0E};

int LEDVal;

pthread_mutex_t buslock = PTHREAD_MUTEX_INITIALIZER;   					// declare mutex to lock data output through output bus
pthread_mutex_t threadlock = PTHREAD_MUTEX_INITIALIZER; 

pthread_cond_t motor_signal = PTHREAD_COND_INITIALIZER; 
pthread_cond_t main_signal = PTHREAD_COND_INITIALIZER; 

void initlcd();
void lcd_writecmd(char cmd);
void LCDprint(char *sptr);
void LCDprint_Line2(char *sptr);
void lcddata(unsigned char cmd);

void motor_func();
void LED_func();

/******************* Threads **********************/

void* thread_LCD(void* value)
{

    int i;    
	while (1)															// loop forever
	{ 	
		usleep(100000);													// delay
		LCDprint_Line2(">>>>>>>>>>>>>>>");                           	// print to LCD
		usleep(100000);
		LCDprint_Line2("               ");
		usleep(100000);
		LCDprint_Line2("<<<<<<<<<<<<<<<");	
	}
    usleep(1000);

}

void* thread_motor(void* value)
{   

	int j;
    int i;
	int steps = 50;                         							// specify amount of steps for stepper motor
	int cmd_count = 3; 													// command count

	while(1)															// loop forever
	{
		pthread_mutex_lock(&threadlock);
		pthread_cond_wait(&main_signal, &threadlock);

		if (cmd_count == 3)
		{
			//LED_func(cmd_count);										// output to LED command number
			motor_func(steps, CW, motor_fast);							// send to motor function number of steps, direction, speed
			cmd_count --;												// reduce command count by 1
		}

		else if (cmd_count == 2)
		{
			//LED_func(cmd_count);
			motor_func(steps, CCW, motor_slower);
			cmd_count --;	
		}

		else if (cmd_count == 1)
		{
			//LED_func(cmd_count);
			motor_func(steps, CW, motor_slowest);
			cmd_count = 3;												// reset command count to 3
		}

		pthread_cond_signal(&motor_signal);
		pthread_mutex_unlock(&threadlock);
	}
 	                         
}

/****************** MAIN PROGRAM *********************/

int main(int argc, char *argv[]) 
{
	CM3DeviceInit();
	
	pthread_t thread1, thread2;											// Declare threads
	int ptr1, ptr2;														// name threads
	int* ptr;
	int cmd_count = 3;  

    initlcd();
	usleep(10000);
	lcd_writecmd(0x80);
	LCDprint("Multitasking Lab"); 										// print to LCD
	usleep(1000);
	
	ptr2 = pthread_create(&thread2,NULL,thread_motor,NULL);				// create and start thread for thread_motor
	usleep(1000);
	ptr1 = pthread_create(&thread1,NULL,thread_LCD,NULL);				// create and start thread for thread_LCD
	
	while(1)
	{
		pthread_mutex_lock(&threadlock);								// enables locking of thread
		if (cmd_count == 3)
		{
			LED_func(cmd_count);
			cmd_count --;	
		}
		else if (cmd_count == 2)
		{
			LED_func(cmd_count);
			cmd_count --;	
		}
		else if (cmd_count == 1)
		{
			LED_func(cmd_count);
			cmd_count = 3;	
		}
		pthread_cond_signal(&main_signal); 									// Sends signal to other threads to proceed
		pthread_cond_wait(&motor_signal, &threadlock); 					 	// wait on motor thread to complete
		pthread_mutex_unlock(&threadlock);
	}



    //pthread_join(thread1,(void**)&ptr);									// wait for thread1 to finish before continuing program
	//pthread_join(thread2,(void**)&ptr);									// wait for thread2 to finish before continuing program

	return 0;
}

//------------ Motor function -------------//

void motor_func(int steps_no, int dir, int speed)
{
	int i;
	int j;
	int motor_dir;

	for(i=0;i<=steps_no;i++)
	{
		
		for(j=0;j<4;j++)
		{
			if (dir == CW) 												// if direction clockwise specified
				{
				motor_dir = clockwise[j];								
				} 
			if (dir == CCW) 											// if direction couter-clockwise specified
				{
				motor_dir = counter_clockwise[j];						
				}
			pthread_mutex_lock(&buslock);								// lock output bus, only stepper motor will receive data        	/***********/           
			CM3_outport(SMPort, motor_dir);								// output to stepper motor											/*IMPORTANT*/
			pthread_mutex_unlock(&buslock);								// unlock output bus, for other adresses to use                     /***********/
			usleep(speed);												// delay set to control speed
		}	   
 	}
}

//------------ LED function -------------//

void LED_func(int cmd_count)
{
	pthread_mutex_lock(&buslock);										// lock output bus, only LED will receive data
	CM3_outport(LEDPort, Bin2LED[cmd_count]);							// output to LED
	pthread_mutex_unlock(&buslock);										// unlock output bus, for other adresses to use
}

//------------ LCD function -------------//

void initlcd(void)														// function to initialise LCD
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

void lcd_writecmd(char cmd)												// function to output hex as LCD commands
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

void LCDprint(char *sptr)												// function to print to LCD
{
	while (*sptr != 0)
	{
		int i=1;
        lcddata(*sptr);
		++sptr;
	}
}

void LCDprint_Line2(char *sptr)											// function used in LCD thread to print to line 2 of LCD
{
	pthread_mutex_lock(&buslock);										// lock output bus, only LCD will receive data                    
	lcd_writecmd(0xC0);													// output LCD command, set to line 2
	pthread_mutex_unlock(&buslock);                                     // unlock output bus, for other adresses to use
	while (*sptr != 0)
	{
		int i=1;
		pthread_mutex_lock(&buslock);									// lock output bus, only LCD will receive data     
        lcddata(*sptr);													// output to LCD 
		pthread_mutex_unlock(&buslock);									// unlock output bus, for other adresses to use
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


