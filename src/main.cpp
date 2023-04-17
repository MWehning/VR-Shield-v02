/*
	VR-Shield Project

	hard and software system that simplifies linking i2c sensors to other systems over bluetooth

	*Required hardware: ESP32-Wroom32 Dev Board C, Custom VR-Shield PCB (v03 or up)
	*Supported sensors: MPU6050, QMC5883L, BH1750 (Attached using VR-Dapter board)

	@author MWehning
	@version 3.0 14/04/2023

*/

bool updateFlag = false;		// TODO: remove scan on startup and replace with scheduled rescan
bool debugflag = true;

#include "bluetooth_handler.h"
#include "v_id_handler.h"

// own ID
#define ID	0x01

// device matrice sizing 
#define SAME_DEVICE_COUNT 5
#define DEVICE_TYPES      4


unsigned long previousMillis;

byte storage[4]; 				// stores current requested address

bool active_processes[DEVICE_TYPES][SAME_DEVICE_COUNT] = { 0 };		
// uses the same grid as vid table, 
// true==process active(data grabbing and filtering)



// ISR for scheduling rescans
void IRAM_ATTR ISR() {
	updateFlag = true;			
}

// Configure boot button for scan func
void attachManualScan(){
	pinMode(GPIO_NUM_0, INPUT); 
	attachInterrupt(GPIO_NUM_0,ISR,FALLING);	// trigger on press
}

void setup()
{
	setupDevices(debugflag);
	printOutMask(debugflag);
	greetings(SetupBluetooth() && SetupSerial(),debugflag);
	attachManualScan();

	previousMillis = millis();
	// Publish(IniPacket());			// TODO: Publish a packet containing all connected devices
}

void loop()
{
	if(Receiver(storage)){							// can "digest" 1 byte per loop
		printPackageContents(storage,debugflag);	// write received adress into storage buffer 
													// if full message received
		if(storage[0]==ID){					// MCU Addr correct?
			float buffer[9]= { 0 };			// empty buffer to fullfil data request [0,0,0,0,0,0,0,0,0]
			byte type = storage[1]>>4;     	// <--Left 4 bits form device type
			byte count= storage[1] & 0xf;  	// -->Right 4 bits form device count
			getData(buffer,type,count);		// get data from adress 							// TODO: Add to active_process table
			Publish(storage,buffer);		// Respond with an echo + requested data			// TODO: if something isnt accessed for a long time remove again
		}
	}
	


	if (millis() - previousMillis > 1000)	// only executed once every second	
	{
		if(updateFlag){						// button was triggered so rescan is scheduled
			updateFlag = false;
			updateDeviceData(debugflag);
			printOutMask(debugflag);
		}
		previousMillis = millis();
	}
}