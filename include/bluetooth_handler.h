#include "BluetoothSerial.h"

extern uint8_t contentsD[5];
extern uint8_t contentsM[5];
extern uint8_t byteR[11];
extern uint8_t error[11];

//Setup
bool SetupBluetooth();
bool SetupSerial();

// Port register management
int Receiver();
void Publish(u_int8_t msg[10]);

//Packet builders
bool PacketBuilder(uint32_t vid, uint8_t id1, uint16_t v1, uint8_t id2=0, uint16_t v2=0, uint8_t id3=0, uint16_t v3=0);
String ErrorPacket(int type);
String IniPacket();

// Packet decoders
bool DecoderD(String pkg);
bool DecoderM(String pkg);

// Debug Tools
void printPackageContents(char Type);