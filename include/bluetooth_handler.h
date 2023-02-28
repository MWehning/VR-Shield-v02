#include "BluetoothSerial.h"

extern String receivedPackage;
extern uint32_t contentsD[6];
extern uint32_t contentsM[12];

//Setup
bool SetupBluetooth();
bool SetupSerial();

// Port register management
int Receiver();
void Publish(String msg);

//Packet builders
String PacketBuilder(uint32_t vid, uint8_t id1, uint16_t v1, uint8_t id2=0, uint16_t v2=0, uint8_t id3=0, uint16_t v3=0);
String ErrorPacket(int type);
String IniPacket();

// Packet decoders
bool DecoderD(String pkg);
bool DecoderM(String pkg);

// Debug Tools
void printPackageContents(char Type);