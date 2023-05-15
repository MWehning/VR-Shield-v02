/*
	*-VR-Shield Project-*

	hard and software system that simplifies linking i2c sensors to other systems over bluetooth

	*Required hardware: ESP32-Wroom32 Dev Board C, Custom VR-Shield PCB (v03 or up)
	*Supported sensors: MPU6050, QMC5883L, BH1750 (Attached using VR-Dapter board)

	@author MWehning
	@version 3.0 22/04/2023

*/

bool updateFlag = false;		
bool debugflag = true;

#include "bluetooth_handler.h"
#include "v_id_handler.h"

// own ID
#define ID	0x01

// device matrice sizing 
#define SAME_DEVICE_COUNT 	5
#define DEVICE_TYPES      	4

#define DATA_RETRIEVAL_RATE	10	//(ms)
#define REL_TIMEOUT			10000 	//(ms)


#define TASK_TIMEOUT		(REL_TIMEOUT/DATA_RETRIEVAL_RATE)


unsigned long previousMillis;

byte storage[4]; 				// stores current requested address
int16_t data_buffer[9]= { 0 };	// empty buffer to fullfil data request [0,0,0,0,0,0,0,0,0]			// int limit

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

void iniMessage(){
	byte ini_addr[] = {ID,0xFF,0xFF};
	int16_t ini_devices[30] = {0};
	iniPacket(ini_devices);
	// TODO: !! MPU0 is 00, so the byte is 0, which stops the transmission when sending the scan message. Will be solved by turning the filler into -1
	Publish(ini_addr,ini_devices);	
	for(int i = 0;i<9;i++){
		data_buffer[i] = 0;						// clean buffer for next use
	}	
	
}

void setup()
{
	enableHardware(debugflag);			// Enable Power for devices and Wire func
	updateFlag = true;					// Schedule device scan
	greetings(SetupBluetooth() && SetupSerial(),debugflag);
	attachManualScan();

	previousMillis = millis();
	
}

void loop()
{
	if(Receiver(storage)){							// can "digest" 1 byte per loop
		printPackageContents(storage,debugflag);	// write received adress into storage buffer 
													// if full message received
		if(storage[0]==ID){							// MCU Addr correct?
			if(storage[1]==0xff){					// request for device list?
				updateDeviceData(debugflag);		// rescan if Unity Scan is pressed
				iniMessage();						// deliver list 
			}else{
				for(int i =0;i<9;i++){
				data_buffer[i]=0;					// TODO: Make this int limit and apply this too resulting functions and other arrays too
			}
			
			byte type = storage[1]>>4;     		// <--Left 4 bits form device type
			byte count= storage[1] & 0xf;  		// -->Right 4 bits form device count

			Serial.println(storage[2]);
			getSensorValues(data_buffer,storage[2],type,count,TASK_TIMEOUT);	//  data  with data type storage[2] requested			
			
			
			//getData(buffer,type,count);		// get data from adress 							
			Publish(storage,data_buffer);		// Respond with an echo + requested data			
			}
			
			
		}
	}
	


	if (millis() - previousMillis > DATA_RETRIEVAL_RATE)	// only executed once every second	
	{
		previousMillis = millis();
		updateTasks();						// counts down task timeout
		executeTasks();				

		//printOutTasks(debugflag);
		if(updateFlag){						// button was triggered so rescan is scheduled
			updateFlag = false;
			updateDeviceData(debugflag);
			//printOutMask(debugflag);
		}
		
	}
}
