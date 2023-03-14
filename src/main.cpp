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
	{
		uint16_t bufferD[] = {contentsD[0], 0x01, 0x02, 0x03, 100, 200, 300};
		if (PacketBuilder(bufferD)) // should be filled with Sensor Data
			Publish(byteR);
		if (debugflag)
			printPackageContents('D');
		break;
	}

	case 'M':
	{
		uint16_t bufferM[] = {contentsM[0], 1, 1, 1, 1, 1, 1};
		if (PacketBuilder(bufferM)) // confirmation message
			Publish(byteR);
		if (debugflag)
			printPackageContents('M');
		break;
	}

	case 'E':
	{
		uint16_t bufferE[] = {0, 0, 0, 0, 0, 0, 0};
		if (PacketBuilder(bufferE)) // Doesnt fit any scheme
			Publish(byteR);
		if (debugflag)
			Serial.println("\nUnknown Packet Type");
		break;
	}
	default:
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