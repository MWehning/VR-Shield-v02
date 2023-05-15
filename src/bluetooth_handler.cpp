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


// Greets or throws error
// @param state Can all com ports be opened?
// @param debugflag Decides if greeting is carried out
// @return either nothing, a greeting or an error state
void greetings(bool state,bool debugflag){
    
    if(state){
        if(debugflag)Serial.println("VR-Shield V3");
    }else{
        for (int i = 0; i < 1000; i++)
        {
            delay(50);
            Serial.println("ERROR");
        }
    }

}



// Publishes a message or packet on Serial as well as Bluetooth
// @param addr[] address to be echoed
// @param msg[] data that will be appended
// @return A message in the correct format on both ports
void Publish(uint8_t addr[],int16_t msg[])
{
    Serial.write(0xaa);
    SerialBT.write(0xaa);
    for (byte i = 0; i < PKGSIZE; i++)  // echo address
    {
        Serial.write(addr[i]);
        SerialBT.write(addr[i]);
    }
    for (int i = 0; msg[i]!=0; i++) // Write everything from msg buffer
    {
        Serial.printf(" %d ",msg[i]);
        byte hi = highByte(msg[i]);
        byte lo = lowByte(msg[i]);
        //Serial.printf("\n0x%02x 0x%02x\n",hi,lo);
        Serial.write(hi);
        SerialBT.write(hi);
        Serial.write(lo);
        SerialBT.write(lo);
    }
    Serial.write(0xaa);
    SerialBT.write(0xaa);
    Serial.write(0x00);
    SerialBT.write(0x00);
}


// *Heart of package decoding
// @param block single byte fed into machine
// @param storage[] buffer to store message in if successful
// @return msgready flag is set if a message is complete
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
        case 0b11: // ! pointer isnt refreshed yet in state chart
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
                storage[i] = received[i];           // *full message received
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
    /*Serial.printf("-----------\n");
    Serial.printf("Block :%02d\n",block);
    Serial.printf("State :%02d\n",state);
    Serial.printf("Pointer :%02d\n",pointer);
    Serial.print("Condition :");
    Serial.println(a,BIN); */
}

// "Feeds" state machine from com ports
// @param storage[] buffer array message will be stored in
// @return true if a message is complete
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
void printPackageContents(byte storage[],bool debugflag)
{
    if(debugflag){
        Serial.printf("\ne:0x%02x:0x%02x:0x%02x:e\n",storage[0],storage[1],storage[2]);
    }
}