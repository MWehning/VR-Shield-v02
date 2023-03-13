#include "BluetoothSerial.h"

const byte MSGLEN = 11;
BluetoothSerial SerialBT;
uint8_t contentsD[5];
uint8_t contentsM[5];
uint32_t contentsR[7];

const byte PKGSIZE = 5;
uint8_t pointer = 0;
byte received[PKGSIZE];
byte block = 0;
byte valid[PKGSIZE];
bool msgready = false;

uint8_t byteR[MSGLEN];
uint8_t error[MSGLEN] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t state = 0;

uint32_t fourBytes2oneInt(char a, char b, char c, char d)
{
    uint32_t buffer;
    // shift everything to it's correct position
    buffer = (byte)a << 24 + (byte)b << 16 + (byte)c << 8 + (byte)d;
    return buffer;
}

bool SetupBluetooth()
{
    if (!SerialBT.begin("VR-Shield v02"))
        return false;
    return true;
}

bool SetupSerial()
{
    Serial.begin(115200);
    return true;
}

// Publishes a message or packet on Serial as well as Bluetooth
void Publish(uint8_t msg[MSGLEN])
{
    for (int i = 0; i < MSGLEN; i++)
    {
        Serial.write(msg[i]);
        SerialBT.write(msg[i]);
    }
}

// Packet builder, assembles array from given values
// based on D-Type Packet Protocol
bool PacketBuilder(uint32_t vid,
                   uint8_t id1, uint16_t v1, // required
                   uint8_t id2 = 0, uint16_t v2 = 0,
                   uint8_t id3 = 0, uint16_t v3 = 0)
{

    contentsR[0] = vid;
    contentsR[1] = id1;
    contentsR[2] = v1;
    contentsR[3] = id2;
    contentsR[4] = v2;
    contentsR[5] = id3;
    contentsR[6] = v3;

    byteR[0] = 255; // Start of Frame('o')
    byteR[1] = vid;
    byteR[2] = id1;
    byteR[3] = v1 / 100; // '100'00
    byteR[4] = v1 % 100; //  100'00'
    byteR[5] = id2;
    byteR[6] = v2 / 100; // '100'00
    byteR[7] = v2 % 100; //  100'00'
    byteR[8] = id3;
    byteR[9] = v3 / 100;  // '100'00
    byteR[10] = v3 % 100; //  100'00'

    return true;
}


String IniPacket()
{
    return ("Still needs to be implemented"); // TODO: IMPLEMENT INI PROTOCOL
}

void SerialReceiver()
{

    if (Serial.available())
    {
        block = Serial.read();
        byte a = 0b00;
        if (block == 255) // escapement byte
            a = a || 0b01;
        if (pointer >= 5) // full message worth of bytes
            a = a || 0b10;

        switch (state)
        {
        case 0:
            pointer = 0;
            switch (a)
            {
            case 10:
                state = 1;
                break;
            default:
                state = 0;
                break;
            }
        case 1:
            switch (a)
            {
            case 00:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            default:
                state = 1;
                break;
            }
        case 2:

            switch (a)
            {
            case 00:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            case 10:
                state = 3;
                break;
            case 11:
                for (int i : received)
                {
                    valid[i] = received[i];
                    state = 0;
                }
                msgready = true;
                Serial.println("new message");
                break;
            default:
                state = 0;
                break;
            }
        case 3:

            switch (a)
            {
            case 10:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            default:
                state = 0;
                break;
            }
        }
        /*  block = Serial.read();
         bool escapement = false;
         if(block==255){
             escapement = true;
         }
         switch (state){
             case 0:                             // SoF
                 if(escapement)
                     state = 1;
                 pointer = 0;
                 break;

             case 1:
                 if(escapement){
                     state = 0;
                     break;
                 }
                 state = 2;                      // start of message
                 received[pointer] = block;
                 pointer ++;
                 break;

             case 2:
                 if(pointer == PKGSIZE){               // message complete
                     if(escapement){
                         for(int i : received){
                             valid[i]=received[i];
                         }
                     }
                     state = 0;
                 }

                 if(escapement){
                     if(Serial.read() == 255){   // check next byte to see if escapement or SoF/EoF
                         received[pointer] = 255;
                         pointer ++;
                     }else{
                         state = 0;              // reset if escapement but not followed with another
                     }
                     break;
                 }
                 received[pointer] = block;      // non escapement chars are always valid
                 pointer ++;
                 break;

             default:
                 state = 0;
                 break;
         } */
    }
}

void BluetoothReceiver()
{
    if (SerialBT.available())
    {
        block = SerialBT.read();
        byte a = 0b00;
        if (block == 255) // escapement byte
            a = a || 0b01;
        if (pointer >= 5) // full message worth of bytes
            a = a || 0b10;

        switch (state)
        {
        case 0:
            pointer = 0;
            switch (a)
            {
            case 10:
                state = 1;
                break;
            default:
                state = 0;
                break;
            }
        case 1:
            switch (a)
            {
            case 00:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            default:
                state = 1;
                break;
            }
        case 2:

            switch (a)
            {
            case 00:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            case 10:
                state = 3;
                break;
            case 11:
                for (int i : received)
                {
                    valid[i] = received[i];
                    state = 0;
                }
                msgready = true;
                Serial.println("new message");
                break;
            default:
                state = 0;
                break;
            }
        case 3:

            switch (a)
            {
            case 10:
                received[pointer] = block;
                pointer++;
                state = 2;
                break;
            default:
                state = 0;
                break;
            }
        }
    }
}

// Reads out packets from the registers assigned to communication,
// returned value shows if there's a new message
int Receiver()
{
    SerialReceiver();
    BluetoothReceiver();
    if(msgready){
        return true;
        msgready = false;
    }else{
        return false;
    }
    
}

bool Decoder(byte input[5]){
    switch (input[0]){
        case 'D':
            contentsD[0]=input[1];
            contentsD[1]=input[2];
            contentsD[2]=input[3];
            contentsD[3]=input[4];
            printPackageContents('D');
            break;
        case 'M':
            contentsD[0]=input[1];
            contentsD[1]=input[2];
            contentsD[2]=input[3];
            contentsD[3]=input[4];
            printPackageContents('M');
            break;
        default:
            Serial.println("Unknown Packet Type");
            for(int i = 0 ; i<5;i++){
                Serial.print((char)input[i]);
            }Serial.println("");
            
            break;
    }
}

// Debug function used to visually represent last received packages(D- or M-Type)
void printPackageContents(char Type)
{

    if (Type == 'D')
    {
        Serial.println("----------------");
        Serial.println("D-Type package");
        Serial.printf("Device:%03d\n", contentsD[0]);
        Serial.printf("Data ID1:%03d\n", contentsD[1]);
        Serial.printf("Data ID2:%03d\n", contentsD[2]);
        Serial.printf("Data ID3:%03d\n", contentsD[3]);

        Serial.println("----------------");
    }
    else if (Type == 'M')
    {
        Serial.println("----------------");
        Serial.println("M-Type package");
        Serial.printf("Device:%03d\n", contentsM[0]);
        Serial.print("Data points: ");
        Serial.printf("%03d,%03d,%03d\n", contentsM[1], contentsM[2], contentsM[3]);

        Serial.println("----------------");
    }
    else if (Type = 'R')
    {
        Serial.println("----------------");
        Serial.println("Responded with");
        Serial.printf("Device:%03d\n", contentsR[0]);
        Serial.printf("Data ID1:%03d\n", contentsR[1]);
        Serial.printf("Value:%03d\n", contentsR[2]);
        Serial.printf("Data ID2:%03d\n", contentsR[3]);
        Serial.printf("Value:%03d\n", contentsR[4]);
        Serial.printf("Data ID3:%03d\n", contentsR[5]);
        Serial.printf("Value:%03d\n", contentsR[6]);
        Serial.println("----------------");
    }
}