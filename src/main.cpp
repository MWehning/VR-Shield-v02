#include "bluetooth_handler.h"
// #include "v_id_handler.h"

unsigned long previousMillis;
byte storage[4];

bool debugflag = true;

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
	switch (Receiver(storage))
	{ // decode received message and put into appropiate values
	case 'D':
		if (PacketBuilder(contentsD[0], 1, 1234, 2, 360)) // should be filled with Sensor Data
			Publish(byteR);
		if (debugflag)
			printPackageContents('D');
		break;
	case 'M':
		if (PacketBuilder(contentsM[0], 1, 1, 1, 1)) // confirmation message
			Publish(byteR);
		if (debugflag)
			printPackageContents('M');
		break;
	case 'E':
		if (PacketBuilder(0, 0, 0, 0, 0)) // Doesnt fit any scheme
			Publish(byteR);
		if (debugflag)
			Serial.println("\nUnknown Packet Type");
		break;
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