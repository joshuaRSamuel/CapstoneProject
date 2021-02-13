		//--- Temperature and Humidity Sensor Final Code ---
/*----------------------------------------------------------------------------------------------
	File Name:	Team8TempHmdSensor_V1.3.c
	Author:		RSparling, JMichael, SMahadik
	Date:		18/04/2020
	Modified:	None
	copyright Fanshawe College, 2020

	Description: 	Take temperature and humidity samples on a four second basis. After one 
					minute, the values are averaged and sent to a ThingSpeak server via ESP8266 
					wifi module.
----------------------------------------------------------------------------------------------*/

// Preprocessor ------------------------------------------------------------------------------

#include "pragmas.h"
#include <stdlib.h>
#include <delays.h>
#include <stdio.h>
#include <p18f45k22.h>
#include "usart.h"
#include <string.h>


// Constants ---------------------------------------------------------------------------------

#define TRUE		1		
#define	FALSE		0	


// Sensor Data Control
#define SENSORDATA 		PORTDbits.RD0
#define SENSORSTATE 	TRISDbits.RD0
#define INPUT			1
#define OUTPUT			0
#define MAXDATA			32		// Bits in a long variable
#define MAXCS			8		// Checksum of DHT is a char
#define LIMIT			40		// DHT22 time interval, lower = 0, higher = 1
#define MAXSAMP			15		// Number of samples taken for average

#define RCFLAG		PIR1bits.RC1IF	// High when a byte is ready to be received

#define ESPBUF 		50				// Buffer size for received string

const char SSID[] =			"Wifi Name";	// Specific to local wifi
const char PASSWORD[] =		"WifiPassword";
const char SERVERIP[] =		"api.thingspeak.com";
const char SERVERPORT[] =	"80";
const char APIKEY[] = 		"XXXXXXXXXXXXXXXX";	// Specific to created thingspeak channel
				 
#define DEG 176	// Degree symbol

// Timer data
#define T0FLAG		INTCONbits.TMR0IF
#define PRESETH		0x0B
#define PRESETL		0xDC				// Preset for 4 second rollover

#define INTERVAL	15					// Number of minutes between average samples (minimum 1)
#define UNITNO		1					// For assigning which server fields to write to (1 = Inside, 2 = Outside)

// Global Variables --------------------------------------------------------------------------

char sensorStatement[ESPBUF];

typedef struct sensor
{
	int tempSamp[MAXSAMP];	// Arrays converted to decimal values
	int hmdSamp[MAXSAMP];
	char insertAt;				// Indexing arrays
	char avgRdy;				// Indicate if average is ready to be taken
	int tempAvg;				// Average values of arrays
	int hmdAvg;
}sensor_t;

sensor_t dht22;	
// Functions ---------------------------------------------------------------------------------


/*** setFOsc: ******************************************************
Author:		RSparling, JMichael 
Date:		24 Jan, 2020		
Modified:	None
Desc:		Sets the internal Oscillator of the Pic 18F45K22 to 16 MHz.
Input: 		None
Returns:	None
**********************************************************************************/
void setFOsc(void)
{
	OSCCON =  			0x72;			// Sleep on slp cmd, HFINT 16 MHz, INT OSC Blk 		
	OSCCON2 = 			0x04;			// PLL No, CLK from OSC, MF off, Sec OSC off, Pri OSC 	
	OSCTUNE = 			0x80;			// PLL disabled, Default factory freq tuning 	
	
	while (OSCCONbits.HFIOFS != 1); 	// wait for osc to become stable
}
//eo: setFOsc:: ***************************************************


/*** portConfig: ******************************************************
Author:		SMAHADIK,JMICHAEL
Date:		26 Jan, 2020		
Modified:	None
Desc:		Configures the port for DHT22 sensor.
Input: 		None
Returns:	None
**********************************************************************************/
void portConfig(void)
{
	// PortD: DHT22 sensor on RD0 (input)
	ANSELD=0x00;	
	LATD=0x01;
	TRISD=0x01;

	// PortC: Set up for serial connection (RC7 is input)
	ANSELC=0x00;	
	LATC=0x00;
	TRISC=0x80;
}  
//eo portConfig:: **********************************************************************


/*** serialCom: ***********************************************************
Author:		RSparling
Date:		24 Jan, 2020
Modified:	None
Desc:		Configures RS232 Connection to 19200 Baud rate, asynchronous
Input: 		None
Returns:	None
**********************************************************************************/
void serialCom(void)
{
	SPBRG1 = 	51;		// Fosc = 16 MHz, 19200 Baud Rate (BRGH = 1, BRG16 = 0)

	RCSTA1 = 	0x90;	// Serial port enabled, 8 bit reception, Receiver enabled

	TXSTA1 = 	0x24;	// 8 bit transmission, Transmit enabled, Asynchronous mode, 
						// Sync Break transmission completed, High Boad Rate speed, TSR Empty

	BAUDCON1 = 	0x40;	// No auto-boad timer overflow, receiver is idle, data is not inverted, 
						// High transmit idle state, no 16-bit Baud Rate generator, receiver 
						//operating normally, auto-baud detect is disabled
	
}	
// eo serialCom::**********************************************************


/*** timer0Reset: ***********************************************************
Author:		RSparling, JMichael
Date:		24 Jan, 2020
Modified:	None
Desc:		Resets timer to desired preset.
Input: 		None
Returns:	None
**********************************************************************************/
void timer0Reset(void)
{
	T0FLAG = FALSE;		// Reset timer rollover flag
	TMR0H = PRESETH;	// Reset for one second flag event (3036)
	TMR0L = PRESETL;
}	
// eo timer0Reset::**********************************************************


/*** initializeSystem: ***********************************************************
Author:		JMichael
Date:		24 Jan, 2020
Modified:	None
Desc:		Initialize PIC, including Fosc, serial connection, and sensor IO.
Input: 		None
Returns:	None
**********************************************************************************/
void initializeSystem(void)
{
	
									
	setFOsc();			// Set the processor speed (4 MHz)

	portConfig();		// Port IO

	serialCom();		// Set up serial connection

	// Timer config
	timer0Reset();
	T0CON = 0x97;		// Initialize timer0 1:256 prescaler (sample timing)
	T1CON = 0x21;		// Initialize timer1 1:8 prescaler (sensor timer reading)

}	
// eo initializeSystem::**********************************************************


/****** initializeDHT:*******************************************************
Author:		JMichael
Date:		02 Apr, 2020
Modified:	None
Desc:		Initializes values within the global object dht22.
Input: 		None
Returns:	None
**********************************************************************************/
void initializeDHT(void)
{
	char index = 0;							// Placeholders for cycling through sample history

	for(index = 0; index < MAXSAMP; index++)		// Cycle through and reset all registered sensor structures
	{
		dht22.tempSamp[index] = 0;
		dht22.hmdSamp[index] = 0;
	}//eo for
	
	dht22.insertAt = 0;
	dht22.avgRdy = FALSE;
	dht22.tempAvg = 0;
	dht22.hmdAvg = 0;
}
// eo initializeDHT::**********************************************************


/*** espGetCh: ***********************************************************
Author:		RSparling
Date:		26 Feb, 2020
Modified:	None
Desc:		Collects and returns character from RCReg1, if hung up, 
			Watchdog timer will reset system
Input: 		None
Returns:	None
**********************************************************************************/
char espGetCh(void)
{
	char hold = 0;	
	// Enable WDT
	WDTCON = TRUE;
						
	while(!RCFLAG);
	hold = RCREG1;		// Return byte from RC register

	// Disable WDT
	WDTCON = FALSE;
	return hold;
}	
// eo espGetCh::**********************************************************


/*** espCheckOK: ***********************************************************
Author:		RSparling
Date:		27 Feb, 2020
Modified:	None
Desc:		Look for "OK" response from ESP wifi, meaning command has been 
			accepted and completed.
Input: 		None
Returns:	None
**********************************************************************************/
void espCheckOK(void)
{	
	char notDone = TRUE;

	// Check RC register for overflow, reset if overrun
	if(RCSTA1bits.OERR)
	{
		RCSTA1bits.CREN = FALSE;
		RCSTA1bits.CREN = TRUE;
	}

	while(notDone)
	{
		if(espGetCh() == 'O')
		{
			if(espGetCh() == 'K')	// If 'O' and 'K' are received in succession
			{						// ESP has accepted and acted on the command
				notDone = FALSE;
			}
		}
	}
}	
// eo espCheckOK::**********************************************************


/*** espInitialize: ***********************************************************
Author:		RSparling
Date:		30 Mar, 2020
Modified:	None
Desc:		Initialize the ESP8266, ensure it is ready to receive data.
Input: 		None
Returns:	None
**********************************************************************************/
void espInitialize(void)
{	
	printf("AT\r\n");
	espCheckOK();		// Ensure proper communication
	
	printf("AT+CWMODE=1\r\n");	// Set to station mode
	espCheckOK();
}	
// eo espInitialize::**********************************************************


/*** espSend: ***********************************************************
Author:		RSparling
Date:		29 Feb, 2020
Modified:	None
Desc:		Send a string to ESP
Input: 		String to be sent to ESP,
			Length of the string
Returns:	None
**********************************************************************************/
void espSend(char *ptr, int len)
{	
	char notDone = TRUE;
	char index = 0;

	printf("AT+CIPSEND=%i\r\n",len);	// Initialize ESP data sending procedure	
	while(notDone)
	{
		if(espGetCh() == '>')					// Wait for '>', meaning ESP is 
		{										// ready for string to be sent
			notDone = FALSE;
		}
	}
	for(index = 0; index < len; index++)	// Send string
	{
		printf("%c",*ptr);
		ptr++;		
	}
	espCheckOK();
}	
// eo espSend::**********************************************************


/*** saveToArray: ******************************************************
Author:		JMichael, RSparling
Date:		02 Apr, 2020		
Modified:	None
Desc:		Save read values to array to be averaged later
Input: 		Integral temperature value, Integral humidity value.
Returns:	None
**********************************************************************************/
void saveToArray(int temp, int humd)
{
	dht22.tempSamp[dht22.insertAt] = temp;
	dht22.hmdSamp[dht22.insertAt] = humd;
	
	dht22.insertAt++;
	if(dht22.insertAt >= MAXSAMP)
	{
		dht22.insertAt = 0;
		dht22.avgRdy = TRUE;
	}
}
// eo saveToArray:: ******************************************************


/*** startSignal: ******************************************************
Author:		CTalbot
Date:		26 Jan, 2020		
Modified:	JMichael, SMahadik
Desc:		Program that sends a start signal from dht22 and initiates sensor.
Input: 		None
Returns:	None
**********************************************************************************/
void startSignal()	// *** modified
{
	SENSORSTATE = OUTPUT;			//making sensor as output device
	SENSORDATA = FALSE;				//sinking dataline to low
	Delay1KTCYx(8);					// Delays for 1ms to start a read process
	SENSORDATA = TRUE;				//sinking dataline to high
	Delay10TCYx(12);				// Delay for 40 us
	//***DHT22 should take over at this point
	SENSORDATA = FALSE;
	SENSORSTATE = INPUT;			//DHT22 configured as input device 
	// End of start sequence
	// DHT22 should take over at this point and pull the line.
	while( !SENSORDATA );
	while( SENSORDATA );
	
} 
//eo startSignal:: *********************************************************


/*** checkResponse: ******************************************************
Author:		CTalbot
Date:		26 Jan, 2020		
Modified:	JMichael, SMahadik
Desc:		Program that checks for a response from dht22 sensor and stores temperature & humidity values.
Input: 		None
Returns:	None
**********************************************************************************/
void checkResponse()
{
	int index = 0;
	long data = 0;
	int humd = 0, temp = 0;
	char checksum = 0;

	// Enable WDT
	WDTCON = TRUE;

	for( index=0; index< MAXDATA; index++)
	{
	
		while( !SENSORDATA );	// Reads time that sensor responds low, then high
		TMR1H=0;
		TMR1L=0;
		while( SENSORDATA );
		
		if( TMR1L > LIMIT )			// If time is higher than specified time, bit is 1.
		{
			data = (data<<1) + 1;
		}
		else if( TMR1L < LIMIT )	// If lower, bit is 0
		{
			data = (data<<1);
		}
	}
	for( index=0; index< MAXCS; index++)	// Same process for checksum
	{
	
		
		while( SENSORDATA );
		TMR1H=0;
		TMR1L=0;
		while( !SENSORDATA );
		
		if( TMR1L > LIMIT )
		{
			checksum = (checksum<<1)+ 1;
		}
		else if( TMR1L < LIMIT )
		{
			checksum = (checksum<<1);
		}
	}

	// Disable WDT
	WDTCON = FALSE;

	temp = data;			// First byte of long is temperature
	humd = (data>>16) ;		// Second byte is humidity

	saveToArray(temp, humd);
	
	Nop();
	SENSORSTATE = OUTPUT;	// Reset IO pin for next time around
	SENSORDATA = TRUE;

	Nop();
}
// eo checkResponse:: ******************************************************


/*** calcAvg: ******************************************************
Author:		JMichael
Date:		02 Apr, 2020		
Modified:	None
Desc:		Calculate average of temperature and humidity arrays
Input: 		None
Returns:	None
**********************************************************************************/
void calcAvg()
{
	int tempSum = 0;
	int hmdSum = 0;
	char index = 0;
	
	for(index = 0; index < MAXSAMP; index++)
	{
		tempSum += dht22.tempSamp[index];
		hmdSum += dht22.hmdSamp[index];
	}
	dht22.tempAvg = tempSum/MAXSAMP;
	dht22.hmdAvg = hmdSum/MAXSAMP;
}
// eo calcAvg:: ******************************************************


/*--- MAIN FUNCTION -------------------------------------------------------------------------
-------------------------------------------------------------------------------------------*/

void main(void)
{
	char stepper = 0;	// When stepper reaches the interval time, sample will be taken
	initializeSystem();	// Initialize PIC and connection with ESP8266 Wifi Module
	initializeDHT();
	espInitialize();

	// Connect to access point
	printf("AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,PASSWORD);
	espCheckOK();

	while(TRUE)
	{
		if(T0FLAG)						// True every four seconds
		{
			timer0Reset();	// Reset timer
			stepper++;

			// Interval based on minutes between average sample
			// If INTERVAL = 1, sample every 4 seconds, average every 1 minute
			// If INTERVAL = 2, sample every 8 seconds, average every 2 minutes
			// And so on...
			if(stepper >= INTERVAL)		
			{							
				stepper = 0;

				startSignal();
				checkResponse();	// Get sensor reading
				
				if(dht22.avgRdy)
				{
					calcAvg();
					dht22.avgRdy = FALSE;

					// Send data
					// Connect to TCP Server
					printf("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",SERVERIP,SERVERPORT);
					espCheckOK();
					
					// Create and send sentence
					sprintf(sensorStatement,"GET /update?api_key=%s&field%i=%i.%i&field%i=%i.%i\r\n",APIKEY,
											(2*UNITNO)-1,dht22.tempAvg/10,dht22.tempAvg%10,
											(2*UNITNO),dht22.hmdAvg/10,dht22.hmdAvg%10);
					espSend(sensorStatement,strlen(sensorStatement));					
				}
			}
		}// eo T0 if
	}// eo while
// eo main	
}