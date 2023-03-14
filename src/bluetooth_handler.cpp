#include "BluetoothSerial.h"

const byte MSGLEN = 11;
BluetoothSerial SerialBT;
uint8_t contentsD[4];
uint8_t contentsM[4];
uint32_t contentsR[7];

const byte PKGSIZE = 4;
uint8_t pointer = 0;
byte received[PKGSIZE];
byte block = 0;
char Type;
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

void StateMachine(byte storage[5])
{
    byte a = 0b00;
    if (block >= 123) // escapement byte
        a = a + 0b10;
    if (pointer >= 4) // full message worth of bytes
        a = a + 0b01;

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
            Type = (char)block;
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
            if (Type == 'D')
            {
                for (int i = 0; i < PKGSIZE; i++)
                {
                    storage[i] = received[i];;
                    contentsD[i] = received[i];; // put M-Contents from buffer to contents Array
                }
            }
            else if (Type == 'M')
            {
                for (int i = 0; i < PKGSIZE; i++)
                {
                    storage[i] = received[i];
                    contentsM[i] = received[i]; // put M-Contents from buffer to contents Array
                }
            }
            else
            {
                for (int i = 0; i < PKGSIZE; i++)
                {
                    storage[i] = received[i]; // just return message, no decoding possible
                }
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
    /* Serial.printf("Block :%02d\n",block);
    Serial.printf("State :%02d\n",state);
    Serial.printf("Pointer :%02d\n",pointer);
    Serial.print("Condition :");
    Serial.println(a,BIN); */
}

// Reads out packets from the registers assigned to communication,
// returned value shows if there's a new message
char Receiver(byte storage[PKGSIZE])
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
        return Type;
    }
}

// Debug function used to visually represent last received packages(D- or M-Type)
void printPackageContents(char Type)
{

    if (Type == 'D')
    {
        Serial.println("\n----------------");
        Serial.println("D-Type package");
        Serial.printf("Device:%03d\n", contentsD[0]);
        Serial.printf("Data ID1:%03d\n", contentsD[1]);
        Serial.printf("Data ID2:%03d\n", contentsD[2]);
        Serial.printf("Data ID3:%03d\n", contentsD[3]);

        Serial.println("----------------");
    }
    else if (Type == 'M')
    {
        Serial.println("\n----------------");
        Serial.println("M-Type package");
        Serial.printf("Device:%03d\n", contentsM[0]);
        Serial.print("Data points: ");
        Serial.printf("%03d,%03d,%03d\n", contentsM[1], contentsM[2], contentsM[3]);

        Serial.println("----------------");
    }
    else if (Type = 'R')
    {
        Serial.println("\n----------------");
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
