#include "BluetoothSerial.h"

const int MSGLEN = 11;
BluetoothSerial SerialBT;
uint8_t contentsD[5];
uint8_t contentsM[5];
uint32_t contentsR[7];
uint8_t PKGbuffer[5];
uint8_t byteR[MSGLEN];
uint8_t error[MSGLEN] = {255,0,0,0,0,0,0,0,0,0,0};

uint32_t fourBytes2oneInt(char a, char b, char c, char d){
    uint32_t buffer;
    // shift everything to it's correct position 
    buffer = (byte)a<<24 + (byte)b<<16 + (byte)c<<8 + (byte)d;      
    return buffer;
}

bool SetupBluetooth() {
    if(!SerialBT.begin("VR-Shield v02"))
        return false;
    return true;
}

bool SetupSerial() {
    Serial.begin(115200);
    return true;
}

// Publishes a message or packet on Serial as well as Bluetooth
void Publish(uint8_t msg[MSGLEN]) {
    for(int i=0;i<MSGLEN;i++){
        Serial.write(msg[i]);
        SerialBT.write(msg[i]);
    }
}

// Packet builder, assembles array from given values
// based on D-Type Packet Protocol
bool PacketBuilder(uint32_t vid, 
                     uint8_t id1,   uint16_t v1,    // required
                     uint8_t id2=0, uint16_t v2=0, 
                     uint8_t id3=0, uint16_t v3=0) {

    contentsR[0]=vid;
    contentsR[1]=id1;
    contentsR[2]=v1;
    contentsR[3]=id2;
    contentsR[4]=v2;
    contentsR[5]=id3;
    contentsR[6]=v3;
    

    byteR[0]= 255;                     // Start of Frame('o')
    byteR[1]= vid;
    byteR[2]= id1;
    byteR[3]= v1/100;                  // '100'00
    byteR[4]= v1%100;      	          //  100'00'
    byteR[5]= id2;
    byteR[6]= v2/100;                  // '100'00
    byteR[7]= v2%100;            //  100'00'
    byteR[8]= id3;
    byteR[9]= v3/100;                  // '100'00
    byteR[10]=v3%100;            //  100'00'

    return true;


}

String ErrorPacket(int type) {
    /* String strbuffer = "";              // TODO: give v_id with error
    byte Empty = '-';
    for(int i=3;i>(PACKETLN-1);i++){    // 0-17 0b00000000
        strbuffer += Empty;
    }
    strbuffer+=type;                    // last digit == error code
    return strbuffer; */
    return ("-------------------");
}

String IniPacket(){
    return("Still needs to be implemented");    // TODO: IMPLEMENT INI PROTOCOL
}
bool SerialReceiver(){
    bool empty = true;
    while(Serial.available()){
        if((byte)Serial.read() > 100){      // flush until a SoF is reached
            empty = false;
            break;
        }
    }
    if(empty){
        return 0;                           // No full message transmitted, error(presumably first half cut off)
    }
    for(int i = 0;i<5;i++){
        if(Serial.available()){
            PKGbuffer[i]+=Serial.read();    // Put last message into buffer(always same size)
        }else{
            return 0;                       // No full message transmitted, error(presumably second half cut off)
        }
    }
    while(Serial.available())
        Serial.read();
    return 1;
}

bool BluetoothReceiver(){
    bool empty = true;
    while(SerialBT.available()){
        if((byte)SerialBT.read() > 100){      // flush until a SoF is reached
            empty = false;
            break;
        }
    }
    if(empty){
        //Serial.println("first half");
        return 0;                           // No full message transmitted, error(presumably first half cut off)
    }
    for(int i = 0;i<5;i++){
        if(SerialBT.available()){
            PKGbuffer[i]+=SerialBT.read();    // Put last message into buffer(always same size)
        }else{
            //Serial.println("second half");
            return 0;                       // No full message transmitted, error(presumably second half cut off)
        }
    }
    while(SerialBT.available())
        SerialBT.read();
    return 1;
}

// Reads out packets from the registers assigned to communication,
// returned value represents the packet type
int Receiver() {
    for(int i=0;i<5;i++){   
        PKGbuffer[i] = 0;        // Empty buffer to receive new message
    }

    if(SerialBT.available()){       
        if(!BluetoothReceiver()){
            return 3;               // ERROR!
        }
    }else if(Serial.available()){
        if(!SerialReceiver()){
            return 3;
        }
    }                               // PKG is now in PKGbuffer or an error was returned
    switch ((char)PKGbuffer[1])
    {
    case 'D':
        contentsD[0]=PKGbuffer[0];  // P_ID
        contentsD[1]=PKGbuffer[2];  // D_ID1
        contentsD[2]=PKGbuffer[3];  // D_ID2
        contentsD[3]=PKGbuffer[4];  // D_ID3
        return 1;                   // D-Type Package
        break;
    case 'M':
        contentsM[0]=PKGbuffer[0];  // P_ID
        contentsM[1]=PKGbuffer[2];  // DP1
        contentsM[2]=PKGbuffer[3];  // DP2
        contentsM[3]=PKGbuffer[4];  // DP3
        return 2;                   // M-Type Package
        break;
    default:
        break;
    }
    return 0;                       //Nothing received
}

// Debug function used to visually represent last received packages(D- or M-Type)
void printPackageContents(char Type) {


    if(Type == 'D'){
        Serial.println("----------------");
        Serial.println("D-Type package");
        Serial.printf("Device:%03d\n",contentsD[0]);
        Serial.printf("Data ID1:%03d\n",contentsD[1]);
        Serial.printf("Data ID2:%03d\n",contentsD[2]);
        Serial.printf("Data ID3:%03d\n",contentsD[3]);
        
        Serial.println("----------------");


    }else if(Type == 'M'){
        Serial.println("----------------");
        Serial.println("M-Type package");
        Serial.printf("Device:%03d\n",contentsM[0]);
        Serial.print("Data points: ");
        Serial.printf("%03d,%03d,%03d\n",contentsM[1],contentsM[2],contentsM[3]);

        Serial.println("----------------");
    }else if(Type = 'R' ){
        Serial.println("----------------");
        Serial.println("Responded with");
        Serial.printf("Device:%03d\n",contentsR[0]);
        Serial.printf("Data ID1:%03d\n",contentsR[1]);
        Serial.printf("Value:%03d\n",contentsR[2]);
        Serial.printf("Data ID2:%03d\n",contentsR[3]);
        Serial.printf("Value:%03d\n",contentsR[4]);
        Serial.printf("Data ID3:%03d\n",contentsR[5]);
        Serial.printf("Value:%03d\n",contentsR[6]);
        Serial.println("----------------");
    }
}