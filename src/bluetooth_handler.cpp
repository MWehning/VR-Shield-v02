#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const int PACKETLN = 19;
String receivedPackage;
uint32_t contentsD[6];
uint32_t contentsM[12];
uint32_t contentsR[7];

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
void Publish(String msg) {
    if(msg.length()>1){
        Serial.println(msg);               
        SerialBT.println(msg);
    }
}

// Packet builder, assembles string from given values 
// based on D-Type Packet Protocol
String PacketBuilder(uint32_t vid, 
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

    char buffer[PACKETLN];
    String strbuffer;
    // Empty bytes are used as seperator
    byte Empty = 0b00000000; 
    // ccc 1 11 0 1 11 0 1 11 0 0 0 0 0 
    
    int i = 0;
    char cvid[3];
    sprintf(cvid,"%03d",vid); //  Ensures theres 3 digits TODO: sprintf takes long
    while(i<3){
        buffer[i]=cvid[i];
        i++;
    }

    buffer[3]=id1;
    buffer[4]=highByte(v1);
    buffer[5]=lowByte(v1);
    buffer[6]= Empty;

    buffer[7]=id2;
    buffer[8]=highByte(v2);
    buffer[9]=lowByte(v2);
    buffer[10]= Empty;

    buffer[11]=id3;
    buffer[12]=highByte(v3);
    buffer[13]=lowByte(v3);

    i=7;
    while(i<PACKETLN){
        buffer[i++]=Empty;
        i++;
    }
    strbuffer = buffer;
    return strbuffer;
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

// Reads out packets from the registers assigned to communication,
// returned value represents the packet type
int Receiver() {
    String buffer = "";
    if(Serial.available()){
        while(Serial.available()){
            buffer += (char)Serial.read();
        }
        receivedPackage = buffer;
            
    }else if(SerialBT.available()){
        while(SerialBT.available()){
            buffer += (char)SerialBT.read();
        }
        receivedPackage = buffer; 
    }

    // identify message type
    if(buffer != ""){
        switch(buffer[3]){
            case 'D':
                return 1;
                break;
            case 'M':
                return 2;
                break;
            default:
                return 3;
                break;
        }
    }
    return 0;
}

bool DecoderD(String pkg) {
    String v_id = pkg.substring(0,3);       // Virtual ID 
    contentsD[0] = v_id.toInt();
    if(contentsD[0]==0)
        return false;                       // TODO: THROW DECICIVE ERROR

    if(pkg[4]=='R'){                        // Data Order or Request?
        contentsD[1]=0;
    }else if(pkg[4]=='O'){
        contentsD[1]=1;
    }else{                                  // TODO: THROW DECICIVE ERROR
        return false;
    }

    contentsD[2]=(byte)pkg[5];              // Data IDs
    contentsD[3]=(byte)pkg[7];
    contentsD[4]=(byte)pkg[9];

    uint32_t interval=                      // '0' isnt == 0 (manual use)
    fourBytes2oneInt(pkg[11],pkg[12],pkg[13],pkg[14]);

    contentsD[5]=interval;                 // interval time

    return true; 
}

bool DecoderM(String pkg) {
    String v_id = pkg.substring(0,3);       // Virtual ID 
    contentsM[0] = v_id.toInt();        
    if(contentsM[0]==0)
        return false;                       // TODO: THROW DECICIVE ERROR
    
    contentsM[1]=(byte)pkg[4];              // data points
    contentsM[2]=(byte)pkg[5];
    contentsM[3]=(byte)pkg[6];
    contentsM[4]=(byte)pkg[7];              
    contentsM[5]=(byte)pkg[8];
    contentsM[6]=(byte)pkg[9];
    contentsM[7]=(byte)pkg[10];              
    contentsM[8]=(byte)pkg[11];
    contentsM[9]=(byte)pkg[12];
    contentsM[10]=(byte)pkg[13];

    for(int i = 1; i<=10;i++){              // ensure Datapoints are <=100
        if(contentsM[i]>100)
            contentsM[i]=100;
    }

    uint32_t interval=                      // '0' isnt == 0 (manual use)
    fourBytes2oneInt(pkg[15],pkg[16],pkg[17],pkg[18]);
    contentsD[11]=interval;                 // interval time

    return true;
}

// Debug function used to visually represent last received packages(D- or M-Type)
void printPackageContents(char Type) {


    if(Type == 'D'){
        Serial.println("----------------");
        Serial.println("D-Type package");
        Serial.printf("Device:%03d\n",contentsD[0]);
        if(contentsD[1]==0){
            Serial.printf("Data Request\n");
        }else if(contentsD[1]==1){
            Serial.printf("Data Order\n");
            Serial.printf("Order Interval:%d\n",contentsD[5]);
        }
        Serial.printf("Data ID1:%03d\n",contentsD[2]);
        Serial.printf("Data ID2:%03d\n",contentsD[3]);
        Serial.printf("Data ID3:%03d\n",contentsD[4]);
        Serial.println("----------------");


    }else if(Type == 'M'){
        Serial.println("----------------");
        Serial.println("M-Type package");
        Serial.printf("Device:%03d\n",contentsM[0]);
        Serial.print("Data points:");
        for(int i=1;i<=9;i++)
            Serial.printf("%d,",contentsM[i]);
        Serial.println(contentsM[10]);
        Serial.printf("Time steps:%d\n",contentsM[11]);
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