#include "bluetooth_handler.h"
// #include "v_id_handler.h"

unsigned long previousMillis;

bool debugflag = false;

void setup()
{
	if (SetupBluetooth() && SetupSerial())
		if (debugflag)
			Serial.println("VR-Shield v2");
	previousMillis = millis();
	// Publish(IniPacket());
}

void loop()
{
	if(Receiver()){			// Receive == true if there's a new valid message
		Decoder(valid);		// decode received message and put into appropiate values
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