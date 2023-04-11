#include "BluetoothSerial.h"

extern uint8_t contentsD[5];
extern uint8_t contentsM[5];
extern uint8_t byteR[11];
extern uint8_t error[11];

//Setup
bool SetupBluetooth();
bool SetupSerial();

// Port register management
bool Receiver(byte storage[]);
void Publish(u_int8_t msg[]);

//Packet builders
bool PacketBuilder(uint16_t contents[]);
String IniPacket();

// Debug Tools
void printPackageContents(byte storage[]);