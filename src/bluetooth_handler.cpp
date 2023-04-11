#include "BluetoothSerial.h"

const byte MSGLEN = 13;
BluetoothSerial SerialBT;

uint32_t contentsR[7];

const byte PKGSIZE = 3;
uint8_t pointer = 0;
uint8_t pointer2 = 2;
byte received[PKGSIZE];
byte block = 0;
char Type;
bool msgready = false;

byte byteR[50];
uint8_t error[MSGLEN] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255};

uint8_t state = 0;

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
void Publish(uint8_t msg[])
{
    for (int i = 0; i <= pointer2; i++)
    {
        Serial.write(msg[i]);
        SerialBT.write(msg[i]);
    }
}

// Packet builder, assembles array from given values
// based on D-Type Packet Protocol
bool PacketBuilder(uint16_t contents[7])
{
    for(int i = 0;i<6;i++){
        contentsR[i] = contents[i];     // vid, ad0, v0, ad1, v1, ad2, v2
    }
    
   

   
    byteR[0]=255;               // SoF
    if(contents[0]<123){
        byteR[1]=(byte)contents[0];   // vid (can't use escapement)
    }else{
        return false;           // error, VID out of range
    }
    pointer2 = 2;
    for(int i = 1;i<4;i++){
        if(contents[i]>123){
            byteR[pointer2]=lowByte(contents[i]); // adresses with a single byte
            pointer2++;
        }
        byteR[pointer2]=lowByte(contents[i]);
        pointer2++;
    }

    for(int i = 4;i<7;i++){             
        if(highByte(contents[i])>123){
            byteR[pointer2]=highByte(contents[i]); // data with two bytes(High byte)
            pointer2++;
        }
        byteR[pointer2]=highByte(contents[i]);
        pointer2++;

        if(lowByte(contents[i])>123){
            byteR[pointer2]=lowByte(contents[i]); // data with two bytes(Low byte)
            pointer2++;
        }
        byteR[pointer2]=lowByte(contents[i]);
        pointer2++;
    }
    byteR[pointer2] = 255;

    return true;
}


void StateMachine(byte storage[])
{
    byte a = 0b00;
    if (block == 0xaa) // escapement byte
        a += 0b10;
    if (pointer >= 3) // full message worth of bytes
        a += 0b01;

    switch (state)
    {
    case 0:
        pointer = 0;
        switch (a)
        {
        case 0b10:
            state = 1;
            break;
        case 0b11: // pointer isnt refreshed yet in state chart
            state = 1;
            break;
        default:
            state = 0;
            break;
        }
        break;
    case 1:
        switch (a)
        {
        case 0b00:
            received[pointer] = block;
            pointer++;
            state = 2;
            break;
        default:
            state = 1;
            break;
        }
        break;
    case 2:

        switch (a)
        {
        case 0b00:
            received[pointer] = block;
            pointer++;
            state = 2;
            break;
        case 0b10:
            state = 3;
            break;
        case 0b11:

            state = 0;

            for (int i = 0; i < PKGSIZE; i++)
            {
                storage[i] = received[i];
                pointer = 0;
            }

            msgready = true;
            break;
  
        default:
            state = 0;
            break;
        }
        break;
    case 3:

        switch (a)
        {
        case 0b10:
            received[pointer] = block;
            pointer++;
            state = 2;
            break;
        default:
            state = 0;
            break;
        }
        break;
    }
    Serial.printf("-----------\n");
    Serial.printf("Block :%02d\n",block);
    Serial.printf("State :%02d\n",state);
    Serial.printf("Pointer :%02d\n",pointer);
    Serial.print("Condition :");
    Serial.println(a,BIN); 
}

// Reads out packets from the registers assigned to communication,
// returned value shows if there's a new message
bool Receiver(byte storage[PKGSIZE])
{
    if (SerialBT.available())
    {
        block = SerialBT.read();
        StateMachine(storage);
    }
    else if (Serial.available())
    {
        block = Serial.read();
        StateMachine(storage);
    }

    if (!msgready)
    {
        return 0;
    }
    else
    {
        msgready = false;
        return 1;
    }
}

// Debug function used to visually represent last received packages(D- or M-Type)
void printPackageContents(byte storage[])
{
    Serial.printf("e:0x%02x:0x%02x:0x%02x:e\n",storage[0],storage[1],storage[2]);
}
