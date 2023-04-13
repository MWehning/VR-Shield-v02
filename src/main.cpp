#include "bluetooth_handler.h"
#include "v_id_handler.h"

#define ID	0x01


unsigned long previousMillis;
byte storage[4];

bool debugflag = false;

void setup()
{
	setupDevices();
	printOutMask();

	if (SetupBluetooth() && SetupSerial())
		if (debugflag)
			Serial.println("VR-Shield v2");
	previousMillis = millis();
	// Publish(IniPacket());
}

void loop()
{
	if(Receiver(storage)){
		printPackageContents(storage);		// write received adress into storage buffer
		if(storage[0]==ID){
			float buffer[9]= { 0 };
			getData(buffer,storage[1],storage[2]);	// get data from adress if MCU ID is correct
			Publish(storage,buffer);				// TODO: Convert float to bytes
			for(byte i = 0;buffer[i]!= 0;i++)
				Serial.printf("%f,",buffer[i]);
	}

	
	}
	
	


	if (millis() - previousMillis > 1000)
	{
		if (!debugflag)
		{
			// Publish(PacketBuilder(16,1,1234,2,360));  // TODO: WHY DOES LEADING ZERO LEAD TO DIFFERENT ID?
			// printPackageContents('R');
		}

		previousMillis = millis();
	}
}