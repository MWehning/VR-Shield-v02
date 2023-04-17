#include "BluetoothSerial.h"

extern uint8_t byteR[11];
extern uint8_t error[11];

//Setup

bool SetupBluetooth();
bool SetupSerial();
void greetings(bool state, bool debugflag);

// Port register management

bool Receiver(byte storage[]);
void Publish(uint8_t addr[],float msg[]);

//Packet builders

bool PacketBuilder(uint16_t contents[]);
String IniPacket();

// Debug Tools

void printPackageContents(byte storage[],bool debugflag);