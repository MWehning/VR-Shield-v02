#include "BluetoothSerial.h"

extern byte valid[5];
extern uint8_t contentsD[5];
extern uint8_t contentsM[5];
extern uint8_t byteR[11];
extern uint8_t error[11];

//Setup
bool SetupBluetooth();
bool SetupSerial();

// Port register management
bool Receiver(byte storage[]);
void Publish(u_int8_t msg[10]);

//Packet builders
bool PacketBuilder(uint32_t vid, uint8_t id1, uint16_t v1, uint8_t id2=0, uint16_t v2=0, uint8_t id3=0, uint16_t v3=0);
String IniPacket();

// Packet decoders
char Decoder(byte input[]);

// Debug Tools
void printPackageContents(char Type);